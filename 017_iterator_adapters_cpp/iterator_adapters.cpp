
#include <iterator>
#include <cstdint>
#include <memory>
#include <iostream>
#include <cassert>
#include <cstring>

// aliases that I like

template <typename T> using Unique = std::unique_ptr<T>;
template <typename T> using Ptr = T*;

template <typename R1, typename R2>
struct Zip;

template <typename R1>
struct Enumerate;

/** base 'trait' class for all of my ScuffedRanges. Doesn't check
 * where the Derived class should be able to call certain functions
 * (like sum()), so... */
template <typename Derived>
struct ScuffedRange {

    /** Don't even check if the inner type returns integers lol */
    int32_t sum() const {
        int32_t acc = 0;
        for (int32_t i : static_cast<const Derived&>(*this)) {
            acc += i;
        }
        return acc;
    }

    template <typename OtherDerived>
    Zip<Derived, OtherDerived> zip(OtherDerived& other) {
        return Zip<Derived, OtherDerived>(static_cast<Derived&>(*this), other);
    }

    Enumerate<Derived> enumerate() {
        return Enumerate<Derived>(static_cast<Derived&>(*this));
    }
};

/** Zips two ranges together. Immediately stops if one range
 * of the two ends */
template <typename R1, typename R2>
struct Zip : ScuffedRange<Zip<R1,R2>> {
private:
    R1& L;
    R2& R;
    
public:
    Zip(R1& l, R2& r) : L(l), R(r) {}

    template <typename I1, typename I2>
    struct ZipIterator {
        I1 l_iter;
        I2 r_iter;

        using value_type = std::pair<typename I1::value_type, typename I2::value_type>;
        using reference_type = std::pair<typename I1::reference_type, typename I2::reference_type>;
        using pointer_type = void;
        using iterator_category = std::input_iterator_tag;

        ZipIterator(I1 l, I2 r) : l_iter(std::move(l)), r_iter(std::move(r)) {}
        
        reference_type operator*() {
            return reference_type(*l_iter, *r_iter);
        }

        ZipIterator& operator++() {
            ++l_iter;
            ++r_iter;
            return *this;
        }

        ZipIterator operator++(int) {
            ZipIterator prev = *this;
            ++(*this);
            return prev;
        }
        
        bool operator==(const ZipIterator& other) const {
            return l_iter == other.l_iter || r_iter == other.r_iter;
        }

        bool operator!=(const ZipIterator& other) const {
            return !(*this == other);
        }
    };

    using iterator = ZipIterator<typename R1::iterator, typename R2::iterator>;

    iterator begin() const {
        return iterator(L.begin(), R.begin());
    }

    iterator end() const {
        return iterator(L.end(), R.end());
    }
};

template <typename R>
struct Enumerate : ScuffedRange<Enumerate<R>> {
private:
    R& L;
public:
    Enumerate(R& l) : L(l) {}

    template <typename I>
    struct EnumerateIterator {
        uint32_t index;
        I iter;

        using value_type = std::pair<uint32_t, typename I::value_type>;
        using reference_type = std::pair<uint32_t, typename I::reference_type>;
        using pointer_type = void;
        using iterator_category = std::input_iterator_tag;

        EnumerateIterator(I p_iter) : iter(p_iter), index(0) {}

        reference_type operator*() {
            return reference_type(index, *iter);
        }

        EnumerateIterator& operator++() {
            ++iter;
            ++index;
            return *this;
        }

        EnumerateIterator operator++(int) {
            EnumerateIterator prev = *this;
            ++(*this);
            return prev;
        }

        bool operator==(const EnumerateIterator& other) const {
            return iter == other.iter;
        }

        bool operator!=(const EnumerateIterator& other) const {
            return !(*this == other);
        }
    };

    using iterator = EnumerateIterator<typename R::iterator>;

    iterator begin() const {
        return iterator(L.begin());
    }

    iterator end() const {
        return iterator(L.end());
    }
};

/** Maybe another day */
// template <typename F, typename R1>
// struct Map {
    // private:
//     F function;
//     R1 L;
// public:
//     Map(const R1& l, F func) : function(func), L(l) {}
// };

struct SliceIterator {
    const Ptr<int32_t> m_data;
    uint32_t m_at;

    using value_type = int32_t;
    using reference_type = value_type&;
    using pointer_type = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    reference_type operator*() {
        return m_data[m_at];
    }
    
    const reference_type operator*() const {
        return m_data[m_at];
    }
    
    pointer_type operator->() {
        return &m_data[m_at];
    }

    SliceIterator& operator++() {
        m_at += 1;
        return *this;
    }
    
    SliceIterator operator++(int) {
        SliceIterator prev = *this;
        ++(*this);
        return prev;
    }

    bool operator==(const SliceIterator& other) const {
        return m_data == other.m_data && m_at == other.m_at;
    }
    
    bool operator!=(const SliceIterator& other) const {
        return !(*this == other);
    }
};

struct Slice : public ScuffedRange<Slice> {
    const Ptr<int32_t> data;
    uint32_t start; uint32_t size;

    Slice(const Ptr<int32_t> data, uint32_t start, uint32_t size)
        : data(data), start(start), size(size) {}
    
    int& operator[](uint32_t i) {
        return data[i + start];
    }
    
    const int32_t& operator[](uint32_t i) const {
        return data[i + start];
    }

    bool operator==(const Slice& other) const {
        return data == other.data && start == other.start && size == other.size;
    }

    using iterator = SliceIterator;

    iterator begin() const {
        return SliceIterator{ data, start };
    }
    
    iterator end() const {
        return SliceIterator{ data, start + size };
    }
};

void debug_print_slice(const Slice& s) {
    std::cout << "Slice: (ptr = " << s.data  << ", s.start = " << s.start << ", size = " << s.size << ")" << std::endl; 
}

void print_slice(const Slice& s) {
    std::cout << "[ ";
    for (int i = 0; i < s.size; i++) {
        std::cout << s[i] << " ";
    }
    std::cout << "]" << std::endl;
}

/** Meant to mimic sliding window */
struct Window1D : ScuffedRange<Window1D> {
private:
    uint32_t window_size; // size of the window
    Slice view; // non-owning view of the input data
    
public:
    Window1D(const uint32_t size, Ptr<int32_t> data, uint32_t data_size) 
        : window_size(size), view({ data, 0, data_size }) {
        assert(size < data_size);
    }

    /** Iterator returning slices to the underlying view */
    struct Window1DIterator {
    private:
        Slice m_view;
        Slice m_window;
        static void shift_in_place(Slice& s, int amount) { s.start += amount; }
    public:

    Window1DIterator(const Slice& view, uint32_t start, uint32_t size) 
        : m_view(view), m_window({view.data, start, size}) {}

        using value_type = Slice;
        using reference_type = value_type;
        using pointer_type = void;
        using iterator_category = std::input_iterator_tag;
        using difference_type = int;

        value_type operator*() const {
            return m_window;
        }
        
        Window1DIterator& operator++() {
            shift_in_place(m_window, 1);
            return *this;
        }

        Window1DIterator operator++(int) {
            Window1DIterator prev = *this;
            ++(*this);
            return prev;
        }

        bool operator==(const Window1DIterator& other) const {
            return m_window == other.m_window && m_view == other.m_view;
        }

        bool operator!=(const Window1DIterator& other) const {
            return !(*this == other);
        }
    };
    
    using iterator = Window1DIterator;

    Window1D::iterator begin() const {
        return Window1DIterator(view, 0, window_size);
    }
    
    Window1D::iterator end() const {
        return Window1DIterator(view, view.size - window_size + 1, window_size);
    }
};

/** please put in a number :( */
void get_input_params(uint32_t& length, uint32_t& window_size) {
    std::cout << "Enter the size of the Buffer: ";
    std::cin >> length;
    
    std::cout << "Enter an (Even) Window Size #: ";
    std::cin >> window_size;
}

int main() {
    uint32_t length;
    uint32_t window_size;

    std::cout << "=========================" << std::endl;
    get_input_params(length, window_size);

    // internal buffers
    const Unique<int32_t[]> arr = Unique<int32_t[]>(new int32_t[length]);
    const Unique<int32_t[]> out = Unique<int32_t[]>(new int32_t[length]);
    
    // views over said buffers
    Slice slice_arr = Slice { arr.get(), 0, length };
    Slice slice_out = Slice { out.get(), 0, length };

    for (std::pair<uint32_t,int32_t&> iv : slice_arr.enumerate()) {
        iv.second = iv.first;
    }

    std::memset(out.get(), 0, sizeof(int32_t) * length); // fill out buffer with zeroes
    
    // create sliding window objects for both
    Window1D win = Window1D(window_size, arr.get(), length);
    Window1D out_win = Window1D(window_size, out.get(), length);
    
    // zip em up, sum of all items in window 1 goes to out window
    for (std::pair<Slice,Slice> c : win.zip(out_win)) {
        c.second[c.second.size / 2] = c.first.sum();
    }
    
    std::cout << "=========================" << std::endl;
    print_slice(slice_arr); // before
    print_slice(slice_out); // result
}