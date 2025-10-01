#include <vector>
#include <algorithm>
#include <iostream>

/** Wrapper around some type T that pairs T with some index i */
template <typename T>
struct Idx {
    int i;
    T item;

    static Idx<T> from(const T& item, int index) {
        return Idx<T> { index, item };
    }
};

/** Comparison function for Idx<T> */
static bool cmp_idx(const Idx<int>& a, const Idx<int>& b) {
    return a.item < b.item;
}

/** Map a vector<int> to a vector<Idx<int>> */
static std::vector<Idx<int>> index_map(const std::vector<int>& v) {
    std::vector<Idx<int>> indexed(v.size());
    for (int i = 0; i < v.size(); i++) {
        indexed[i] = Idx<int>::from(v[i], i);
    }
    return indexed;
}

/** TwoSum */
std::vector<int> twoSum(std::vector<int>& nums, int target) {
    std::vector<Idx<int>> idxed = index_map(nums);
    std::sort(idxed.begin(), idxed.end(), cmp_idx);

    int left = 0;
    int right = nums.size() - 1;
    int sum = idxed[left].item + idxed[right].item;

    while (sum != target && left < right) {
        if (sum > target) {
            right -= 1;
        } else if (sum < target) {
            left += 1;
        }

        sum = idxed[left].item + idxed[right].item;
    }

    return {idxed[left].i, idxed[right].i};
}

/** Container representing our twoSum arguments */
struct TwoSumArgs {
    std::vector<int> nums;
    int target;
};

int main() {
    const int target = 13;
    std::vector<TwoSumArgs> tests = {
        { {2,7,11,15}, 9 },
        { {3,2,4}, 6 },
        { {3,3}, 6}
    };

    for (TwoSumArgs& arg : tests) {
        std::vector<int> result = twoSum(arg.nums, arg.target);
        std::cout << "[" << result[0] << "," << result[1] << "]" << std::endl;
    }
}