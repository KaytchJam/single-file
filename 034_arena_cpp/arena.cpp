#include <cstdint>
#include <new>
#include <memory>
#include <iostream>
#include <vector>

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using byte_t = int8_t;

/** Simple "Bumping" memory pool */
struct BumpArena {
    private:
        byte_t* buffer = nullptr;
        u64 total_bytes;
        u64 offset;
    public:
        explicit BumpArena(u64 bytes) {
            buffer = (byte_t*) ::operator new(sizeof(byte_t) * bytes);
            total_bytes = bytes;
            offset = 0;
        }

        ~BumpArena() {
            ::operator delete(buffer);
        }

        using value_type = byte_t;
        using pointer = byte_t*;
        using size_type = u64;

        pointer allocate(size_type n, size_type alignment) {
            size_type space = total_bytes - offset;
            void* alloc_start = static_cast<void*>(buffer + offset);
            void* data_start = std::align(alignment, n, alloc_start, space);

            if (data_start == nullptr || static_cast<byte_t*>(data_start) + n >= buffer + total_bytes) {
                throw std::bad_alloc();
            }
            
            offset +=  (size_type) (static_cast<byte_t*>(data_start) + n - (buffer + offset)); 
            std::cout << "Offset: " << offset << std::endl;
            return (byte_t*) data_start;
        }

        BumpArena& reset() {
            offset = 0;
            return *this;
        }
};

template <typename T>
struct BumpArenaAllocator {
    private:
        BumpArena* arena;
    public:
        using value_type = T;

        explicit BumpArenaAllocator(BumpArena* arena) : arena(arena) {}

        template <typename U>
        BumpArenaAllocator(const BumpArenaAllocator<U>& alloc) : arena(alloc.arena) {} 

        T* allocate(size_t n) {
            return reinterpret_cast<T*>(arena->allocate(n * sizeof(T), alignof(T)));
        }

        void deallocate(T*, size_t) noexcept {}

        bool operator==(const BumpArenaAllocator& other) const noexcept {
            return arena == other.arena;
        }

        bool operator!=(const BumpArenaAllocator& other) const noexcept {
            return !(*this == other);
        }
};

template <typename T>
T* alloc(BumpArena& arena, T&& t) {
    T* ptr = reinterpret_cast<T*>(arena.allocate(sizeof(T), alignof(T)));
    *ptr = std::move(t);
    return ptr;
}

template <typename T>
T* alloc(BumpArena& arena) {
    T* ptr = reinterpret_cast<T*>(arena.allocate(sizeof(T), alignof(T)));
    return ptr;
}

// Fixed Size Case
template <typename T, std::size_t S = 0>
struct ArrHelper {
    using type = T[S];
};

// Unsized Case
template <typename T>
struct ArrHelper<T, 0> {
    using type = T[];
};

// Unified Case
template <typename T, std::size_t S = 0>
using Arr = typename ArrHelper<T,S>::type;

template <typename T>
using Ptr = T*;

template <typename T>
struct Node {
    T item;
    union {
        struct {
            Node* left;
            Node* right;
        };
        Node* children[2];
    };
};

/** T is assumed to be ordinal */
template <typename T, typename Allocator = std::pmr::polymorphic_allocator<T>>
struct BinaryTree {
    private:
        Allocator allocator;
        Node<T>* root = nullptr;

        Node<T>* create_node(T&& value, Node<T>* left = nullptr, Node<T>* right = nullptr) {
            Node<T>* node = allocator.allocate(1);
            new (&node->item) T(std::move(value));
            node->left = left;
            node->right = right;
            return node;
        }
    public:
        using value_type = T;
        using node_type = Node<T>;
    
        BinaryTree(const Allocator& alloc = Allocator()) : allocator(alloc) {}

        BinaryTree& add(T&& val) {
            if (root == nullptr) {
                root = create_node(std::move(val));
                return *this;
            }

            // tree traversal - we know parent can never be nullptr here
            Node<T>* cur = root;
            Node<T>* parent = nullptr;
            while (cur != nullptr) {
                parent = cur;
                cur = cur->children[val >= cur->item];
            }

            parent->children[val >= parent->item] = create_node(std::move(val));
            return *this;
        }
        
        inline Node<T>* get_root() { return root; }

        bool contains(const T& val) {
            Node<T>* cur = root;
            while (cur != nullptr && cur->item != val) {
                cur = cur->children[val >= cur->item];
            }
            return cur != nullptr;
        }
};

template <typename BinaryTreeType>
struct BinaryTreeTraits {
    static_assert(sizeof(BinaryTreeType) == 0, "Specialize BinaryTreeTraits for your tree type");
};

template <typename T, typename Alloc>
struct BinaryTreeTraits<BinaryTree<T, Alloc>> {
    using value_type = typename BinaryTree<T, Alloc>::value_type;
    using node_type = typename BinaryTree<T, Alloc>::node_type;

    inline static value_type& value(BinaryTree<T, Alloc>& tree, node_type& n) { return n.item; }
    inline static node_type* get_root(BinaryTree<T, Alloc>& tree) { return tree.get_root(); }
    inline static node_type* left(BinaryTree<T, Alloc>& tree, node_type& n) { return n.left; }
    inline static node_type* right(BinaryTree<T, Alloc>& tree, node_type& n) { return n.right; }
};

template <typename NodeType>
using VisitNode = std::pair<NodeType*,bool>;

template <typename TreeType>
using DefaultScratch = std::allocator<VisitNode<typename BinaryTreeTraits<TreeType>::node_type>>;

/** Non-owning tree traversal struct. Requires BinaryTreeTraits to be defined for input
 * type TreeType. A scratch allocator may also be passed in. */
template <typename TreeType, typename Scratch = DefaultScratch<TreeType>>
struct InOrderTraversal {
    using node_type = typename BinaryTreeTraits<TreeType>::node_type;
    using value_type = typename BinaryTreeTraits<TreeType>::value_type;
    using BTT = BinaryTreeTraits<TreeType>;

    TreeType& tree;
    std::vector<VisitNode<node_type>, Scratch> stack;
    
    InOrderTraversal(TreeType& tree, const Scratch& alloc = Scratch()) 
        : tree(tree), stack(alloc) {
        node_type* root = BTT::get_root(tree);
        if (root) {
            stack.push_back({ root, false });
        }
    }
    
    /** Advance to the next node in the BinaryTree of TreeType */
    const value_type& next() {
        VisitNode<node_type>& vn = stack.back();
        node_type* out = vn.first;
        
        if (!vn.second) {
            vn.second = true;
            while (node_type* left = BTT::left(tree, *out)) {
                stack.push_back({ left, true });
                out = left;
            }
        }
        
        stack.pop_back();
        node_type* right = BTT::right(tree, *out);
        if (right) {
            stack.push_back({ right, false });
        }
        
        return BTT::value(tree, *out);
    }
    
    /** Check if the traversal can be continued. Returns true if `next` can be safely called,
     * and `false` if not. */
    bool has_next() const {
        return stack.size() > 0;
    }
};

template <typename TreeType, typename Scratch = std::pmr::polymorphic_allocator<VisitNode<typename BinaryTreeTraits<TreeType>::node_type>>>
InOrderTraversal(TreeType&, const Scratch&) -> InOrderTraversal<TreeType,Scratch>;

int main() {
    const u64 NUM_BYTES = 4192;
    BumpArena arena(NUM_BYTES);
    BumpArenaAllocator<i32> my_allocator(&arena);

    // Since we reserve here, we can prevent std::vector from making unecessary allocations
    // during the push-back phase
    std::vector<i32, BumpArenaAllocator<i32>> data(my_allocator);
    data.reserve(100);

    for (i32 i = 0; i < 100; i++) {
        data.push_back(i);
    }

    std::cout << "[ ";
    for (i32 i : data) {
        std::cout << i << " ";
    }
    std::cout << "]" << std::endl;

    // Allocation happens here, 22 bytes are requested from the BumpArena
    BumpArenaAllocator<char> char_allocator(&arena);
    std::basic_string<char, std::char_traits<char>, BumpArenaAllocator<char>> words("Welcome to the jungle", char_allocator);

    std::cout << words << std::endl;

    // The nice thing about 
    i32* test = alloc(arena, 25);

    BumpArenaAllocator<Node<i32>> node_allocator(&arena);
    BinaryTree<i32, BumpArenaAllocator<Node<i32>>> tree(node_allocator);
    Arr<i32> items = {56, 32, 62, 44, 88, 58, 28};
    for (i32 i : items) {
        tree.add(std::move(i));
    }

    for (i32 i : {5, 10, 11, 15, 18, 20, 22, 32, -1, 8}) {
        std::cout << "i = " << i << " is " << (tree.contains(i) ? "" : "NOT ") << "present in the tree" << std::endl;
    }
    
    std::printf("\nTree Traversal: \n");

    i32 j = 1;
    InOrderTraversal traverse(tree);
    while (traverse.has_next()) {
        const i32 val = traverse.next();
        std::printf("#%d - %d\n", j, val);
        j += 1;
    }

    return 0;
}