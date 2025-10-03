#include <vector>
#include <algorithm>
#include <limits>
#include <string>
#include <iostream>

const int INTEGER_MAX = std::numeric_limits<int>::max();

/** Variant of twoSun --- twoSumClosest --- that returns the closest combination to target in our set */
int twoSumClosest(const std::vector<int>& nums, int left, int right, int target) {
    int best_sum = INTEGER_MAX;
    int sum = INTEGER_MAX;

    while (left < right && sum != target) {
        sum = nums[left] + nums[right];

        // are we closer than the BEST
        if (best_sum == INT_MAX || abs(target - sum) < abs(target - best_sum)) {
            best_sum = sum;
        }

        // move closer to the target
        if (sum < target) {
            left += 1;
        } else {
            right -= 1;
        }
    }

    return best_sum;
}
    
/** Implementation of ThreeSumClosest */
int threeSumClosest(std::vector<int>& nums, int target) {
    std::sort(nums.begin(), nums.end());

    const int N = nums.size();
    int closest = INTEGER_MAX;
    for (int i = 0; i < N - 2; i++) {
        const int local_target = target - nums[i];
        const int local_closest = twoSumClosest(nums, i+1, N-1, local_target) + nums[i];

        if (closest == INTEGER_MAX || abs(target - local_closest) < abs(target - closest)) {
            closest = local_closest;
        }
    }

    return closest;
}

/** Arguments necessary for every ThreeSumClosest test */
struct ThreeSumClosestArgs {
    std::vector<int> nums;
    int target;
};

namespace utils {
    std::string vector_to_string(const std::vector<int>& v) {
        const int N = v.size();
        std::string buffer;
        buffer.reserve(N * 2 + 3); // if all our integers are 1 digit then this is enough. If more then :(

        buffer.push_back('[');
        buffer.push_back(' ');
        for (const int i : v) {
            buffer.append(std::to_string(i));
            buffer.push_back(' ');
        }
        buffer.push_back(']');
        return buffer;
    }
}

int main() {
    std::vector<ThreeSumClosestArgs> tests = {
        { {-1,2,1,-4}, 1},
        { {0,0,0}, 1 },
        { {20, 15, 3, -5, -10, 2, 5, 8, 40}, 42}
    };

    int enumeration = 1;
    for (ThreeSumClosestArgs& test : tests) {
        std::cout <<  "TEST (" << enumeration << "): { nums: " << utils::vector_to_string(test.nums) << ", target: " << test.target << " }" << std::endl;
        const int result = threeSumClosest(test.nums, test.target);
        std::cout << "Closest Combination: " << result << ", Error: " << (result - test.target) << "\n" << std::endl;
        enumeration += 1;
    }
}