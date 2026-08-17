#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace std;

struct Interval {
    int32_t start;
    int32_t end;
};

bool sort_by_start(const Interval& a, const Interval& b) {
    return a.start <= b.start;
}

bool sort_by_end_to_start(const Interval& a , const Interval& b) {
    return a.start < b.end;
}

void print_interval(const Interval& i) {
    std::cout << "(" << i.start << "," << i.end << ")"; 
}

void print_intervals(const std::vector<Interval>& ivals, const char* title = nullptr) {
    if (title != nullptr) {
        std::cout << title << ": ";
    }

    for (const Interval& i : ivals) {
        print_interval(i);
        std::cout << " ";
    }

    std::cout << std::endl;
}

Interval merge(const Interval& a, const Interval& b) {
    return Interval{
        std::min(a.start, b.start),
        std::max(a.end, b.end)
    };
}

inline int32_t first_non_overlapping_interval(const std::vector<Interval>& ivals, const Interval& candidate) {
    return std::distance(
        ivals.begin(), std::lower_bound(
            ivals.begin(), ivals.end(), candidate.end, [](const Interval& other, const int32_t end) {
                return other.start < end;
            }
        )
    );
}

typedef std::vector<Interval>::iterator IntervalIter;

int main() {
    std::vector<Interval> ivals = {{10, 25}, {20, 30}, {25, 35}, {40, 45}, {30, 41}, {50, 60}};

    // create our result buffer. capacity if our upper bound
    std::vector<Interval> result;
    result.reserve(ivals.size());

    print_intervals(ivals, "INPUT");
    std::sort(ivals.begin(), ivals.end(), sort_by_start);
    print_intervals(ivals, "SORT");

    int32_t lower = 0;
    while (lower != (int32_t) ivals.size()) {
        Interval candidate = ivals[lower];
        bool candidate_fin = false;

        while (!candidate_fin) {
            const int32_t bound = first_non_overlapping_interval(ivals, candidate);
            int32_t ptr = bound;

            while (ptr != lower) {
                ptr -= 1;
                candidate = merge(candidate, ivals[ptr]);
            }

            lower = bound;
            // no overlap occured. we can confidently push result
            if ((bound - ptr) < 1) {
                candidate_fin = true;
                result.push_back(candidate);
            }

        }
    }

    print_intervals(result, "RESULT");
    std::cout << "Goodbye world" << std::endl;

    return 0;
}