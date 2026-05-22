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

enum class NodeType : uint8_t {
    Dummy,
    Data
};

template <typename T>
struct Node {
    T item;
    Node* left;
    Node* right;
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
                cur = val >= cur->item ? cur->right : cur->left;
            }

            if (val >= parent->item) {
                parent->right = create_node(std::move(val));
            } else {
                parent->left = create_node(std::move(val));
            }

            return *this;
        }

        bool contains(const T& val) {
            Node<T>* cur = root;
            while (cur != nullptr && cur->item != val) {
                cur = val >= cur->item ? cur->right : cur->left;
            }
            return cur != nullptr;
        }
};

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

// 3. The single, unified public alias
template <typename T, std::size_t S = 0>
using Arr = typename ArrHelper<T,S>::type;

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
    Arr<i32> items = {10, 5, 15, 11, 18};
    for (i32 i : items) {
        tree.add(std::move(i));
    }

    for (i32 i : {5, 10, 11, 15, 18, 20, 22, -1, 8}) {
        std::cout << "i = " << i << " is " << (tree.contains(i) ? "" : "NOT ") << "present in the tree" << std::endl;
    }

    return 0;
}