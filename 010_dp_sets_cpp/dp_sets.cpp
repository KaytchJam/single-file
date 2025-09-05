#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

namespace tch {
    void new_lines(size_t count) {
        for (size_t i = 0; i < count; i++) {
            std::cout << std::endl;
        }
    }

    template <typename>
    struct FnHelper;

    template <typename Ret, typename... Args>
    struct FnHelper<Ret(Args...)> {
        using type = Ret(*)(Args...);
    };

    template <typename Signature>
    using Fn = typename FnHelper<Signature>::type;

    template <typename Cont>
    void print_container(const Cont& c) {
        for (auto it = c.cbegin(); it != c.cend(); it++) {
            std::cout << (*it) << " ";
        }
        std::cout << std::endl;
    }

    template <typename Num>
    struct interval {
        Num start;
        Num end;

        inline static bool in_range(Num target, Num lb, Num ub) {
            return lb <= target && target <= ub;
        }

        inline bool overlap(const interval<Num>& other) const {
            return not in_range(start, other.start, other.end) 
                && not in_range(end, other.start, other.end);
        }

        inline static bool compare_start(const interval<Num>& a, const interval<Num>& b) {
            return a.start < b.start;
        }

        inline static bool compare_end(const interval<Num>& a, const interval<Num>& b) {
            return a.end < b.end;
        }

        // this just seemed cool imma be honest
        // returns a lambda that in turn compares the chosen member field of interval<Num>
        inline static auto compare_member(Num interval<Num>::* member) {
            return [member](const interval<Num>& a, const interval<Num>& b) { return a.*member < b.*member; };
        }

        friend std::ostream& operator<<(std::ostream& os, const interval<Num>& ival) {
            return os << "(" << ival.start << "," << ival.end << ")";
        }
    };

    // template <typename A>
    // concept arithmetic = std::is_arithmetic_v<A>;
}

namespace algo {
    typedef tch::interval<int> ival;

    std::vector<size_t> get_last_intervals(const std::vector<ival>& v) {
        using ConstIter = std::vector<ival>::const_iterator;
        std::vector<size_t> ptrs(v.size());

        size_t ival_idx = 0;
        for (const ival& iv : v) {
            ConstIter it = std::lower_bound(v.begin(), v.end(), ival{iv.start, iv.start}, ival::compare_end);
            size_t idx = std::distance(v.begin(), it);
            ptrs[ival_idx] = idx - 1; // wrap-around when idx = 0
            ival_idx += 1;
        }

        return ptrs;
    }

    struct SResult {
        size_t set_size;
        bool uses_cur;

        friend std::ostream& operator<<(std::ostream& os, const SResult& r) {
            return os << "[" << r.set_size << "," << r.uses_cur << "]";
        }
    };

    std::vector<ival> recover_schedule(std::vector<ival>& v, std::vector<size_t>& indices, std::vector<SResult>& sols) {
        size_t read_idx = sols.size() - 1;
        std::vector<ival> sched(sols[read_idx].set_size);
        size_t insert_idx = sched.size() - 1; // insert items in our new list from the back

        // size_t::max() indicates that the interval started its set
        while (read_idx != std::numeric_limits<size_t>::max()) {
            if (sols[read_idx].uses_cur) {
                sched[insert_idx] = v[read_idx];
                insert_idx -= 1;
                read_idx = indices[read_idx];
            } else {
                read_idx -= 1;
            }
        }

        return sched;
    }

    // template <typename Num> requires tch::arithmetic<Num>
    std::vector<ival> interval_scheduling(std::vector<tch::interval<int>>& v) {
        const size_t N = v.size();
        std::sort(v.begin(), v.end(), ival::compare_member(&ival::end));
        std::vector<size_t> indices = get_last_intervals(v);
        std::vector<SResult> solutions(N);

        solutions[0] = { 0, false };
        for (int i = 0; i < N; i++) {
            if (indices[i] == std::numeric_limits<size_t>::max()) {
                solutions[i] = SResult { 1, true };
            } else {
                const size_t prev_set_size = solutions[i-1].set_size;
                const size_t include_set_size = solutions[indices[i]].set_size + 1;
                solutions[i] = SResult { std::max(prev_set_size, include_set_size), prev_set_size <= include_set_size };
            }
        }

        return recover_schedule(v, indices, solutions);
    }

    size_t longest_increasing_subseq(const std::vector<int>& v) {
        std::vector<size_t> solutions(v.size());
        size_t largest = 0;
        solutions[0] = 1;

        for (size_t cur_idx = 1; cur_idx < solutions.size(); cur_idx++) {
            size_t local_max = 1;

            for (size_t prev_idx = cur_idx - 1; prev_idx >= 0 && prev_idx != std::numeric_limits<size_t>::max(); prev_idx--) {
                const bool i_is_greater_than = v[cur_idx] >= v[prev_idx];
                local_max = i_is_greater_than ? std::max(local_max, solutions[prev_idx] + 1) : local_max;
            }

            solutions[cur_idx] = local_max;
            largest = std::max(local_max, largest);
        }

        return largest;
    }

    template <typename Num>
    size_t subset_sum() {
        return 0;
    }
}

// Just having a bit of fun here
template <typename Struct, typename T>
T& project_member(Struct& obj, T Struct::* member) {
    return obj.*member;
}

struct Foo {
    int a;
    int b;
    int c;
    char d;
    bool e;
};

int main() {
    // algorithm 1: interval scheduling
    //tch::new_lines(2);
    std::vector<algo::ival> v = {{4,6}, {8,11}, {3,5}, {1,5}, {1,2}, {5, 9}, {7,9}, {11, 13}};
    tch::print_container(v);

    std::vector<algo::ival> result = algo::interval_scheduling(v);
    std::cout << "largest non-overlapping interval set size = " << result.size() << std::endl;
    tch::print_container(result);

    // algorithm 2: longest increasing subsequence
    tch::new_lines(1);
    std::vector<int> w = {0, -3, -1, 3, 4, -1 };
    tch::print_container(w);

    size_t longest = algo::longest_increasing_subseq(w);
    std::cout << "longest increasing subsequence size = " << longest << std::endl;

    // This stuff below is just for fun
    tch::new_lines(1);
    Foo f = {
        10,
        45,
        80,
        'a',
        true
    };

    int Foo::* member_ptrs[3] = { &Foo::a, &Foo::b, &Foo::c };
    tch::Fn<int& (Foo&, int Foo::*)> project_foo = project_member<Foo,int>;

    for (int Foo::* ptr : member_ptrs) {
        std::cout << project_foo(f, ptr) << std::endl;
    }

    auto lam = [&f](int a) {
        std::cout << a * f.a << std::endl;
        return f.a;
    };

    return EXIT_SUCCESS;
}