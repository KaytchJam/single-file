#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <random>
#include <chrono>
#include <unordered_set>

// continuation of my recap of algorithm notes. revisiting the selection algorithm

namespace tch {
    // slice. does not own its data.
    template <typename T>
    struct slice {
        std::vector<T>* m_parent;
        size_t m_start;
        size_t m_size;

        using iterator = typename std::vector<T>::iterator;
        using const_iterator = typename std::vector<T>::const_iterator;
        
        // slice() : m_parent(nullptr), m_start(0), m_size(0) {}
        
        // slice(std::vector<T>* v, size_t a_start, size_t a_size)
        // : m_parent(v), m_start(a_start), m_size(a_size) {}
        
        // nevermind
        // slice(slice<T>&& other) 
        // : m_parent(other.m_parent), m_start(other.m_start), m_size(other.m_size) {
        //     other.m_parent = nullptr;
        //     m_start = 0;
        //     m_size = 0;
        // }
    
        static slice<T> from(std::vector<T>& v, size_t a_start, size_t a_size) {
            return slice {
                &v,
                a_start,
                a_size
            };
        }

        static slice<T> from(std::vector<T>& v, size_t a_start) {
            return slice {
                &v,
                a_start,
                v.size() - a_start
            };
        }

        static slice<T> from(std::vector<T>& v) {
            return slice {
                &v,
                0,
                v.size()
            };
        }

        // center this slice on a different std::vector<T>
        slice<T>& move_to(std::vector<T>& v) noexcept {
            m_parent = &v;
            return *this;
        }
    
        inline T& operator[](size_t idx) {
            return (*m_parent)[m_start + idx];
        }
    
        inline const T& operator[](size_t idx) const {
            return (*m_parent)[m_start + idx];
        }
    
        inline size_t size() const noexcept {
            return m_size;
        }

        iterator begin() {
            return ((*this).begin()) + m_start;
        }

        iterator end() {
            return ((*this).begin()) + m_start + m_size;
        }
        
        const_iterator cbegin() const {
            return ((*m_parent).cbegin()) + m_start;
        }

        const_iterator cend() const {
            return ((*m_parent).cbegin()) + m_start + m_size;
        }
    };
}

template <typename C>
void print_container(const C& c) {
    for (auto it = c.cbegin(); it != c.cend(); it++) {
        std::cout << (*it) << " ";
    }
    std::cout << std::endl;
}

template <typename I>
void print_range(I&& it, const I& end) {
    for (it; it != end; it++) {
        std::cout << (*it) << " ";
    }
    std::cout << std::endl;
}

template <typename O>
O selection_simple(const std::vector<O>& v, size_t k) {
    std::vector<O> w(v);
    std::sort(w.begin(), w.end());
    return w[k];
}

template <typename O>
const O* find_smallest(const std::vector<O>& v) {
    const O* smallest = nullptr;
    for (const O& o : v) {
        if (smallest == nullptr || o < (*smallest)) {
            smallest = &o;
        }
    }

    return smallest;
}

template <typename O>
const O* find_next_smallest(const std::vector<O>& v, const O* prev_smallest) {
    const O* next_smallest = nullptr;
    for (const O& o : v) {
        const bool is_null = next_smallest == nullptr;
        if ((is_null && (o > (*prev_smallest))) || (!is_null && o < (*next_smallest) && o > (*prev_smallest))) {
            next_smallest = &o;
        }
    }

    return next_smallest;
}

/** Does k passes through the array, updating the smallest item found each pass.
 * The input std::vector can have duplicates.
 */
template <typename O>
O selection_naive(const std::vector<O>& v, size_t k) {
    size_t kth = 0;
    const O* kth_item = find_smallest(v);
    while (kth < v.size() && kth < k) {
        kth_item = find_next_smallest(v, kth_item);
        kth += 1;
    }

    return (*kth_item);
}

/** Integer divison that goes to the ceiling instead of the floor */
inline size_t ceil_div(size_t num, size_t denom) {
    return (num + denom - 1) / denom;
}

/** Returns the median of some slice of Ordinal elements s */
template <typename O>
O median_of_medians(const tch::slice<O>& s) {
    size_t num_buckets = ceil_div(s.size(), 5);
    
    std::array<O, 5> bucket;
    std::vector<O> bucket_means(s.cbegin(), s.cend());
    tch::slice<O> bm_view = tch::slice<O>::from(bucket_means);
    
    
    // Iterate till we have ONE median value left
    size_t iter = 1;
    while (bm_view.size() > 1) {
        size_t insert_idx = 0;
        size_t bucket_idx = 0;
        size_t read_idx = 0;
        const size_t num_vals = bm_view.size();
        while (read_idx < num_vals) {
            // Since `idx` "travels faster" than `bucket_idx`
            // we don't have to worry about overwriting values
            // in the array before we read them
            if (bucket_idx == 5) {
                std::sort(bucket.begin(), bucket.end());
                bm_view[insert_idx] = bucket[2];
                
                bucket_idx = 0;
                insert_idx += 1;
            }
            
            bucket[bucket_idx] = bm_view[read_idx];
            bucket_idx += 1;
            read_idx += 1;
        }
        
        // handle the last bucket
        std::sort(bucket.begin(), bucket.begin() + bucket_idx);
        
        bm_view[insert_idx] = bucket[ceil_div(bucket_idx, 2) - 1];
        bm_view = tch::slice<O>::from(bucket_means, 0, num_buckets);
        num_buckets = ceil_div(num_buckets, 5);
    }
    
    return bm_view[0];
}

/** Result type from a pivot operation */
template <typename T>
struct PivotResult {
    tch::slice<T> view;
    size_t pivot_index;
};

/** Applies the pivot algorithm to some slice<O> s where O is an Ordinal type. */
template <typename O>
PivotResult<O> pivot(tch::slice<O>&& s, const O& pivot_val) {
    const size_t SIZE = s.size();
    size_t i = 0;
    size_t j = 0;
    size_t pivot_index = 0;
    
    // find the pivot's index
    while (pivot_index < SIZE && s[pivot_index] != pivot_val) { pivot_index += 1; }
    std::swap(s[SIZE - 1], s[pivot_index]);
    
    // do the partial sort
    while (j < SIZE - 1) {
        if (s[j] < pivot_val) {
            std::swap(s[j], s[i]);
            i += 1;
        }
        j += 1;
    }
    
    // put the pivot in its proper place
    std::swap(s[SIZE - 1], s[i]);
    return PivotResult<O> { s, i };
}

/** The selection algorithm */
template <typename Ord>
Ord selection_linear(const std::vector<Ord>& v, size_t k) {
    std::vector<Ord> w(v); // dupe. don't want to alter the original - O(N)
    tch::slice<Ord> subarray = tch::slice<Ord>::from(w);
    
    while (subarray.size() > 1) {
        const Ord median = median_of_medians(subarray);
        PivotResult<Ord> res = pivot(std::move(subarray), median);
        const size_t pivot_real = res.pivot_index + res.view.m_start;
        
        // EQUAL TO K: we found the kth smallest
        if (pivot_real == k) {
            return res.view[res.pivot_index];

        // GREATER THAN K: explore the left side of the pivot
        } else if (pivot_real > k) {
            subarray = tch::slice<Ord>::from(
                w, 
                res.view.m_start, 
                pivot_real - res.view.m_start
            );

        // LESS THAN K: explore the right side of the pivot
        } else {
            subarray = tch::slice<Ord>::from(
                w, 
                pivot_real + 1, 
                res.view.m_start + res.view.m_size - (pivot_real + 1)
            );
        }
    }

    return subarray[0];
}

/** The selection algorithm but we're allowed to modify the input std::vector */
template <typename Ord>
Ord selection_linear_mut(std::vector<Ord>& v, size_t k) {
    tch::slice<Ord> subarray = tch::slice<Ord>::from(v);
    
    while (subarray.size() > 1) {
        const Ord median = median_of_medians(subarray);
        PivotResult<Ord> res = pivot(std::move(subarray), median);
        const size_t pivot_real = res.pivot_index + res.view.m_start;
        
        // EQUAL TO K: we found the kth smallest
        if (pivot_real == k) {
            return res.view[res.pivot_index];

        // GREATER THAN K: explore the left side of the pivot
        } else if (pivot_real > k) {
            subarray = tch::slice<Ord>::from(
                v, 
                res.view.m_start, 
                pivot_real - res.view.m_start
            );

        // LESS THAN K: explore the right side of the pivot
        } else {
            subarray = tch::slice<Ord>::from(
                v, 
                pivot_real + 1, 
                res.view.m_start + res.view.m_size - (pivot_real + 1)
            );
        }
    }

    return subarray[0];
}

template <typename T>
bool selection_test(const std::vector<T>& v) {
    std::vector<T> sv = std::vector<T>(v);
    std::sort(sv.begin(), sv.end());
    bool all_true = true;
    
    for (int k = 0; k < v.size(); k++) {
        const T selection_result = selection(v, k);
        const T index_result = sv[k];
        const bool outcome = selection_result == index_result;
        
        std::cout << "SEARCHING k = " << k << std::endl;
        std::cout << "EXPECTED: " << selection_result << std::endl;
        std::cout << "ACTUAL: " << index_result << std::endl;
        std::cout << (outcome ? "CORRECT\n" : "INCORRECT\n") << std::endl;
        
        all_true = all_true && outcome;
    }
    
    return all_true;
}

template <typename numeric>
std::vector<numeric> random_std_vector(const size_t elems, const numeric lower, const numeric upper) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<numeric> dis(lower, upper);
	
	std::vector<numeric> V(elems);
	for (size_t i = 0; i < elems; i++) {
		V[i] = dis(gen);
	}

	return V;
}

template <>
std::vector<int> random_std_vector<int>(const size_t elems, const int lower, const int upper) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(lower, upper);
	
	std::vector<int> V(elems);
	for (size_t i = 0; i < elems; i++) {
		V[i] = dis(gen);
	}

	return V;
}


std::vector<size_t> random_std_vector_no_dupes(const size_t elems) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, elems * 10);
    std::unordered_set<size_t> bag;
	
	std::vector<size_t> V(elems);
	for (size_t i = 0; i < elems; i++) {
        int rng_out = dis(gen);
        while (bag.find(rng_out) != bag.end()) {
            rng_out = dis(gen);
        }

        bag.emplace(rng_out);
        V[i] = rng_out;
	}

	return V;
}

// What if we want to return a reference to the object in the list
// and not a copy of it?
// 
// one approach: make a vector of pointers to items in the original vector
// or some wrapper type that tracks indices.

int main() {
    using std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using std::chrono::duration;
	using std::chrono::milliseconds;

    constexpr size_t size = 100'000;
    std::vector<size_t> v = random_std_vector_no_dupes(size);
    const size_t k = size / 2ull;

    // all the following assuming the vector has NO DUPLICATES (except for select_naive which works for both!)
    std::cout << "random vector (no duplicates) of size N = " << size << std::endl;

    {
        const auto t1 = high_resolution_clock::now();
        const size_t kth_smallest = selection_simple(v, k);
        const auto t2 = high_resolution_clock::now();

        const auto ms_int = duration_cast<milliseconds>(t2 - t1);
        std::cout << "\nselection_simple took " << ms_int.count() << "ms\n";
        std::cout << "kth smallest value where [k = " << k << "]: " << kth_smallest << std::endl;
    }
    
    {
        const auto t1 = high_resolution_clock::now();
        const size_t kth_smallest = selection_linear(v, k);
        const auto t2 = high_resolution_clock::now();

        const auto ms_int = duration_cast<milliseconds>(t2 - t1);
        std::cout << "\nselection_linear took " << ms_int.count() << "ms\n";
        std::cout << "kth smallest value where [k = " << k << "]: " << kth_smallest << std::endl;
    }

    {
        const auto t1 = high_resolution_clock::now();
        const size_t kth_smallest = selection_naive(v, k);
        const auto t2 = high_resolution_clock::now();

        const auto ms_int = duration_cast<milliseconds>(t2 - t1);
        std::cout << "\nselection_naive took " << ms_int.count() << "ms\n";
        std::cout << "kth smallest value where [k = " << k << "]: " << kth_smallest << std::endl;
    }
    
    {
        const auto t1 = high_resolution_clock::now();
        const size_t kth_smallest = selection_linear_mut(v, k);
        const auto t2 = high_resolution_clock::now();

        const auto ms_int = duration_cast<milliseconds>(t2 - t1);
        std::cout << "\nselection_linear_mut took " << ms_int.count() << "ms\n";
        std::cout << "kth smallest value where [k = " << k << "]: " << kth_smallest << std::endl;
    }


    return EXIT_SUCCESS;
}