#include <vector>
#include <algorithm>
#include <optional>
#include <sstream>
#include <string>
#include <iostream>

/** Structure modelling the result expected
 * from a `has_two_sum` function call. */
struct TwoSumResult {
    bool valid;
    int left;
    int right;
};

/** Runs an algorithm on a sorted array of integers `nums` that checks if 
 * some sum of two numbers in indices [left, right] equals target.
 * 
 * Passes the result into `TwoSumResult*` r, where `r->valid` equals true if `nums[r->left] + nums[r->right] = target`
 * and `r.left < r.right`. */
TwoSumResult has_two_sum(const std::vector<int>& nums, int left, int right, int target) {
    int sum = nums[left] + nums[right];
    while (sum != target && left < right) {
        if (sum > target) {
            right -= 1;
        } else if (sum < target) {
            left += 1;
        }
        sum = nums[left] + nums[right];
    }

    return TwoSumResult {
        sum == target && left < right,
        left,
        right
    };
}

/** First implementation of threeSum that uses function n stuff. Think
 * it's easier to understand the algorithm personally. */
std::vector<std::vector<int>> threeSumImpl1(std::vector<int>& nums) {
    const size_t N = nums.size();
    std::sort(nums.begin(), nums.end());
    std::vector<std::vector<int>> solutions;

    for (int i = 0; i < N - 2; i++) {
        const int target = -1 * nums[i];
        if (i > 0 && nums[i] == nums[i-1]) continue;

        int offset = i + 1;
        int rightset = N - 1;
        while (offset < N - 1) {
            const TwoSumResult res = has_two_sum(nums, offset, rightset, target);
            if (res.valid) {
                solutions.push_back({nums[i], nums[res.left], nums[res.right]}); // move ownership of the memory buffer to the vector
            }

            offset = res.left + 1;
            rightset = res.right;
            while (offset < rightset && nums[offset] == nums[offset-1]) offset += 1;
        }
    }

    return solutions;
}

/** The same as threeSumImpl1 but with the function calls and additional
 * types removed. */
std::vector<std::vector<int>> threeSumImpl2(std::vector<int>& nums) {
    const size_t N = nums.size();
    std::sort(nums.begin(), nums.end());
    std::vector<std::vector<int>> solutions;

    for (int i = 0; i < N - 2; i++) {
        const int target = -1 * nums[i];
        if (i > 0 && nums[i] == nums[i-1]) continue;

        int left = i + 1;
        int right = N - 1;
        while (left < right) {
            const int sum = nums[left] + nums[right];
            if (sum == target) {
                solutions.push_back({nums[i], nums[left], nums[right]});
                while (left < right && nums[left+1] == nums[left]) left += 1;
                left += 1;
            } else if (sum < target) {
                left += 1;
            } else {
                right -= 1;
            }
        }
    }

    return solutions;
}

std::string vector_to_string(const std::vector<int>& v) {
    std::vector<int>::const_iterator it = v.cbegin();
    std::stringstream ss;

    ss << "[";
    if (it != v.cend()) { ss << (*it); ++it; }
    for (; it != v.cend(); ++it) {
        ss << ", " << (*it);
    }
    ss << "]";
    return ss.str();
}

int main() {
    std::vector<std::vector<int>> tests = {
        {-1,0,1,2,-1,-4},
        {0,1,1},
        {0,0,0},
        {-100,-70,-60,110,120,130,160}
    };

    std::optional<std::vector<std::vector<int>>> solution = std::nullopt;
    for (std::vector<int>& test : tests) {
        std::cout << "----------------------\nTestcase = " << vector_to_string(test) << "\n==================" << std::endl;

        solution = threeSumImpl1(test);
        std::cout << "Impl1: " << std::endl;
        if (solution.value().size() == 0) {
            std::cout << "N/A";
        } else {
            for (const std::vector<int>& triple : solution.value()) {
                std::cout << "\t" << vector_to_string(triple) << std::endl;
            }
        }
        std::cout << std::endl;

        solution = threeSumImpl2(test);
        std::cout << "====================\nImpl2: " << std::endl;
        if (solution.value().size() == 0) {
            std::cout << "N/A";
        } else {
            for (const std::vector<int>& triple : solution.value()) {
                std::cout << "\t" << vector_to_string(triple) << std::endl;
            }
        }
        std::cout << std::endl;
    }
}