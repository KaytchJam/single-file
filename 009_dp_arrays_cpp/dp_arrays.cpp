#include <iostream>
#include <vector>
#include <limits>

template <typename Cont>
void print_cont(Cont& c) {
    for (auto it = c.begin(); it != c.end(); it++) {
        std::cout << (*it) << " ";
    }
    std::cout << std::endl;
}

int largest_jump_naive(const std::vector<int>& v) {
    int N = (int) v.size();
    int largest = std::numeric_limits<int>::min();

    for (int i = 0; i < N; i++) {
        for (int j = i; j < N; j++) {
            largest = std::max(largest, v[j] - v[i]);
        }
    }

    return largest;
}

int largest_jump_simple(const std::vector<int>& v) {
    int N = (int) v.size();
    int largest = 0;
    int cur_min = v[0];

    for (int i = 1; i < N; i++) {
        cur_min = std::min(cur_min, v[i]);
        largest = std::max(largest, v[i] - cur_min);
    }

    return largest;
}

int sum_range(const std::vector<int>& v, int start_idx, int end_idx) {
    int acc = 0;
    for (int i = start_idx; i < end_idx; i++) {
        acc += v[i];
    }

    return acc;
}

int lss_naive(const std::vector<int>& v) {
    int largest = std::numeric_limits<int>::min();
    for (int i = 0; i < v.size(); i++) {
        for (int j = i + 1; j < v.size(); j++) {
            largest = std::max(largest, sum_range(v, i, j));
        }
    }

    return largest;
}

int lss_dp(const std::vector<int>& v) {
    int solution = v[0];
    int largest = v[0];

    for (int i = 1; i < v.size(); i++) {
        solution = std::max(solution + v[i], v[i]);
        largest = std::max(largest, solution);
    }

    return largest;
}

int main() {
    std::vector<int> v = {9, 2, 3, -8, 5};
    int r1 = largest_jump_naive(v);
    int r2 = largest_jump_simple(v);

    print_cont(v);
    std::cout << "largest_jump_naive: " << r1 << std::endl;
    std::cout << "largest_jump_simple: " << r2 << std::endl;

    r1 = lss_naive(v);
    r2 = lss_dp(v);

    std::cout << "lss_naive: " << r1 << std::endl;
    std::cout << "lss dp: " << r2 << std::endl;

    return EXIT_SUCCESS;
}