#include <iostream>
#include <vector>
#include <limits>

// REVIEWING THE DYNAMIC PROGRAMMING SECTION FROM CLRS

using namespace std;

struct CutRodMemoized {
    vector<int> memory;
    
    int cut_rod_rec(vector<int>& prices, int n) {
        if (n == 0) { return 0; }
        if (memory[n-1] != -1) { return memory[n-1]; }
        
        int best = -1;
        for (int i = 0; i < n; i++) {
            best = max(best, prices[i] + cut_rod_rec(prices, n-1-i));
        }
        
        memory[n-1] = best;
        return best;
    }
    
    int operator()(vector<int>& prices, int n) {
        memory = vector<int>(n, -1);
        return cut_rod_rec(prices, n);
    }
};

struct CutRodBottomUp {
    vector<int> memory;

    int operator()(vector<int>& prices, int n) {
        if (n == 0) { return 0; }
        memory = vector<int>(n, -1);

        memory[0] = prices[0];
        for (int i = 1; i < n; i++) {
            int local_best = -1;
            for (int j = 0; j <= i; j++) {
                local_best = max(local_best, prices[j] + (i-j-1 >= 0 ? memory[i-j-1] : 0));
            }
            memory[i] = local_best;
        }

        return memory[n-1];
    }
};

struct CutRodBottomUpExtended {
    vector<int> memo;
    vector<int> sizes;

    struct CutRodSolution {
        vector<int> sizes;
        int maximum_value;
        int n;
    };

    template <typename F>
    static const CutRodSolution& map_cuts(const CutRodSolution& solution, F func) {
        int rod_size = solution.n - 1;
        while (rod_size > 0) {
            const int cut_size = solution.sizes[rod_size];
            func(cut_size + 1);
            rod_size = rod_size - (cut_size + 1);
        }
        return solution;
    }

    CutRodSolution operator()(vector<int>& prices, int n) {
        if (n == 0) { return { vector<int>(), 0, 0 }; }
        memo = vector<int>(n,-1);
        sizes = vector<int>(n, -1);

        memo[0] = prices[0];
        for (int i = 1; i < n; i += 1) {
            int local_best = -1;
            int size_best = -1;

            for (int j = 0; j <= i; j += 1) {
                const int outcome = prices[j] + (i-j-1 >= 0 ? memo[i-j-1] : 0);
                if (local_best < outcome) {
                    local_best = outcome;
                    size_best = j;
                }
            }

            memo[i] = local_best;
            sizes[i] = size_best;
        }

        return CutRodSolution{
            std::move(sizes),
            memo[n-1],
            n
        };
    }
};

int cut_rod(vector<int>& prices, int n) {
    if (n == 0) { return 0; }
    int best = -1;
    for (int i = 0; i < n; i++) {
        best = max(best, prices[i] + cut_rod(prices, n - 1 - i));
    }
    return best;
}


void cut_rod_tests() {
    int n = 8;
    vector<int> prices = {1, 5, 8, 9, 10, 17, 17, 20, 24, 30};
    
    int expected = 9;
    int actual = cut_rod(prices, n);
    
    CutRodMemoized crm;
    int test_crm = crm(prices, n);
    
    CutRodBottomUp crbu;
    int test_crbu = crbu(prices, n);
    
    CutRodBottomUpExtended crbue;
    CutRodBottomUpExtended::CutRodSolution test_crbue = crbue(prices, n);
    
    cout << "Printing Pieces: ";
    CutRodBottomUpExtended::map_cuts(test_crbue, [](const int cut_size) {
        cout << cut_size << " ";
    });
    cout << endl;
    
    for (int i : crm.memory) {
        cout << i << " ";
    }
    cout << endl;
    
    printf("Expected = %d, Actual = %d\n", expected, actual);
    printf("Expected = %d, Actual (CRM) = %d\n", expected, test_crm);
    printf("Expected = %d, Actual (CRBU) = %d\n", expected, test_crbu);
    printf("Expected = %d, Actual (CRBUE) = %d\n", expected, test_crbue.maximum_value);
}

struct MatrixDimension {
    int rows;
    int cols;
};

int cost(const MatrixDimension a, const MatrixDimension b) {
    return a.rows * a.cols * b.cols;
}

MatrixDimension dimension(vector<int>& p, int start_matrix, int end_matrix_inclusive) {
    return MatrixDimension {
        p[start_matrix],
        p[end_matrix_inclusive+1]
    };
}

MatrixDimension dimension(vector<int>& p, int start_matrix) {
    return MatrixDimension {
        p[start_matrix],
        p[start_matrix+1]
    };
}

// matrix A0 has dimensions p[0] * p[1], A1 has dimensions p[1] * p[2], matrix Ai has p[i] * p[i+1]
// Return the minimum cost of multiplications
int matrix_chain_multiplication(vector<int> p, int start, int end) {
    if (start == end) { return 0; }

    int min_cost = std::numeric_limits<int>::max();
    for (int k = start; k < end; k += 1) {
        const int choice = p[start] * p[k+1] * p[end+1] + matrix_chain_multiplication(p,start,k) + matrix_chain_multiplication(p,k+1,end);
        min_cost = min(min_cost, choice);
    }

    return min_cost;
}

void matrix_mul_tests() {
    vector<vector<int>> p = {{10,100,5,50}, {10,100,5}, {30,35,15,5,10,20,25}};
    for (vector<int>& dims : p) {
        const int minimum_multiplications = matrix_chain_multiplication(dims, 0, dims.size() - 2);
        printf("Minimum: %d\n", minimum_multiplications);
    }
}

struct LCSTopDown {
    std::vector<std::vector<int>> memo;

    int lcs_rec_top_down(const string& t1, const string& t2, int i, int j) {
        if (i < 0 || j < 0) { return 0; }
    
        if (memo[i][j] != -1) { return memo[i][j]; }
    
        int lcs = -1;
        if (t1[i] == t2[j]) {
            lcs = lcs_rec_top_down(t1, t2, i-1, j-1) + 1;
        } else {
            lcs = max(
                lcs_rec_top_down(t1, t2, i-1, j), 
                lcs_rec_top_down(t1, t2, i, j-1)
            );
        }
    
        memo[i][j] = lcs;
        return lcs;
    }

    int longestCommonSubsequence(string text1, string text2) {
        memo = std::vector<std::vector<int>>(text1.size(), std::vector<int>(text2.size(), -1));
        return lcs_rec_top_down(text1, text2, text1.size() - 1, text2.size() - 1);
    }
};

int lcs_bottom_up(const string& t1, const string& t2) {
    std::vector<std::vector<int>> table = std::vector<std::vector<int>>(t1.size(), std::vector<int>(t2.size(), 0));

    const int M = (int) t1.size();
    const int N = (int) t2.size();

    // handle lowest levels for both strings
    table[0][0] = (int) (t1[0] == t2[0]);

    for (int j = 1; j < N; j++) {
        table[0][j] = max(table[0][j-1], (int) (t2[j] == t1[0]));
    }

    for (int i = 1; i < M; i++) {
        table[i][0] = max(table[i-1][0], (int) (t1[i] == t2[0]));
    }

    // handle the rest
    for (int i = 1; i < M; i++) {
        for (int j = 1; j < N; j++) {
            if (t1[i] == t2[j]) {
                table[i][j] = 1 + table[i-1][j-1];
            } else {
                table[i][j] = max(table[i-1][j], table[i][j-1]);
            }
        }
    }

    return table[M-1][N-1];
}

int main() {
    cut_rod_tests();
    matrix_mul_tests();
    return 0;
}