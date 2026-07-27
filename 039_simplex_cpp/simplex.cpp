#include <cstdint>
#include <iostream>
#include <vector>
#include <sstream>
#include <limits>

template <typename T>
using Ptr = T*;

/** Non-owning row-view over a contiguous 2d array of floats. */
struct MatrixRowSlice {
    const Ptr<float> _ptr;
    const size_t _cols;

    MatrixRowSlice(float* src = nullptr, size_t cols = 0) : _ptr(src), _cols(cols) {}

    float& operator[](size_t i) { return *(_ptr + i); }
    const float& operator[](size_t i) const { return *(_ptr + i); }

    MatrixRowSlice& scale(float scalar) {
        for (size_t i = 0; i < _cols; i++) (*this)[i] *= scalar;
        return *this;
    }

    MatrixRowSlice& add(const MatrixRowSlice& other) {
        for (size_t i = 0; i < _cols; i++) (*this)[i] += other[i];
        return *this;
    }
};

/** Non-owning column-view over a contiguous 2d array of floats. */
struct MatrixColSlice {
    const Ptr<float> _ptr;
    const size_t _rows;

    MatrixColSlice(float* src = nullptr, size_t rows = 0) :
        _ptr(src),
        _rows(rows) {}
    
    float& operator[](size_t i) { return *(_ptr + i * _rows); }
    const float& operator[](size_t i) const { return *(_ptr + i * _rows); }
    
    MatrixColSlice& scale(float scalar) {
        for (size_t i = 0; i < _rows; i++) (*this)[i] *= scalar;
        return *this;
    }

    MatrixColSlice& add(const MatrixColSlice& other) {
        for (size_t i = 0; i < _rows; i++) (*this)[i] += other[i];
        return *this;
    }
};

struct Matrix {
    std::vector<float> _buffer;
    size_t _rows = 0;
    size_t _cols = 0;

    Matrix(size_t rows = 0, size_t cols = 0, float initial = 0.f) : _buffer(rows * cols, initial), _rows(rows), _cols(cols) {}
    Matrix(Matrix&& in) : _buffer(std::move(in._buffer)), _rows(in._rows), _cols(in._cols) {}
    Matrix(const Matrix& m) : _buffer(m._buffer), _rows(m._rows), _cols(m._cols) {}

    MatrixRowSlice operator[](const size_t i) noexcept {
        return MatrixRowSlice(
            this->_buffer.data() + (i * this->_rows), 
            this->_cols
        );
    }

    MatrixRowSlice operator()(const size_t i) noexcept { 
        return (*this)[i]; 
    }

    MatrixRowSlice static slice_row(Matrix& m, const size_t row) {
        return m[row];
    }

    float& operator()(const size_t row, const size_t col) noexcept {
        return this->_buffer[row * this->_cols + col];
    }

    const float& operator()(const size_t row, const size_t col) const noexcept {
        return this->_buffer[row * this->_cols + col];
    }

    static std::string print(Matrix& m) {
        std::stringstream ss;
        for (size_t i = 0; i < m._rows; i++) {
            for (size_t j = 0; j < m._cols; j++) {
                ss << m(i, j) << " ";
            }

            if (i != m._rows - 1) {
                ss << "\n";
            }
        }

        return ss.str();
    }
};

namespace LP {

    /** 
     * On Matrix structure:
     * ==
     * (1) Last column is assumed to be the constraints
     * (2) The last `num_slack` columns before the constraints are assumed to be the flex variables
     * (3) The bottom row is assumed to be our objective function
     *  */
    class Simplex {
        private:
            Matrix _tableau;
            Matrix _solution;
        public:
            Simplex(Matrix&& equations, size_t num_slack) : 
                _tableau(std::move(equations)), 
                _solution(1, equations._cols - 1 - num_slack, -std::numeric_limits<float>::infinity()) {}
        private:

            /** Returns the pivot column. If no such column, returns -1. */
            static int32_t find_pivot_column(const Matrix& tableau) {
                const size_t last_row = tableau._rows - 1;
                size_t lowest = 0;
                
                // scan the values in our 'objective' row. the most negative column
                // becomes our pivot column.
                for (size_t j = 1; j < tableau._cols; j++) {
                    const float cur = tableau(last_row, j);
                    if (cur < 0 && cur < tableau(last_row, lowest)) {
                        lowest = j;
                    }
                }

                // edge case because we start at index 1 in our loop
                return tableau(last_row, lowest) < 0 ? static_cast<int32_t>(lowest) : -1;
            }

            /** Given a pivot column, find the pivot row. */
            static int32_t find_target_row(const Matrix& tableau, const size_t pivot) {
                const size_t last = tableau._cols - 1;
                float smallest_quotient = std::numeric_limits<float>::infinity();
                int32_t smallest = -1;

                for (size_t i = 0; i < tableau._rows - 1; i++) {
                    const float quotient = tableau(i, last) / tableau(i, pivot);
                    if (quotient <= smallest_quotient) {
                        smallest = static_cast<int32_t>(i);
                        smallest_quotient = quotient;
                    }
                }

                return smallest;
            }

            /** Given `size_t pivot_col` and `size_t pivot_row`, turns the column into a 
             * pivot column with a 1 on the specified pivot row. */
            Matrix& pivot_at(Matrix& tableau, const size_t pivot_col, const size_t pivot_row) {
                const float frac = 1.f / tableau(pivot_row, pivot_col);
                for (size_t j = 0; j < tableau._cols; j++) {
                    tableau(pivot_row, j) *= frac;
                }

                for (size_t i = 0; i < tableau._rows; i++) {
                    const float factor = -1.f * tableau(i, pivot_col);
                    for (size_t j = 0; i != pivot_row && j < tableau._cols; j++) {
                        tableau(i, j) += tableau(pivot_row, j) * factor;
                    }
                }

                return tableau;
            }

            /** Fancy floating-point equality with a threshold. */
            static bool equal_eps(const float a, const float b, const float eps = 1e-3f) {
                return std::abs(a - b) <= eps;
            }

            /** Returns whether a `size_t column` of `Matrix& m` is a pivot or not. If so, returns 
             * the index of the pivot. Otherwise, -1 is returned. */
            static int32_t is_pivot(const Matrix& m, const size_t column, const float eps = 1e-3f) {
                size_t num_zeroes = 0;
                size_t num_ones = 0;
                int32_t pivot_row = -1;

                for (size_t i = 0; i < m._rows; i++) {
                    const float cur = m(i, column);
                    if (equal_eps(cur, 1.f)) {
                        num_ones += 1;
                        pivot_row = static_cast<int32_t>(i);
                    } else if (equal_eps(cur, 0.f)) {
                        num_zeroes += 1;
                    }
                }

                return num_zeroes == m._rows - 1 && num_ones == 1 ? pivot_row : -1;
            }

            /** Returns a solution to the linear programming maximization problem given
             * the current state of the input `Matrix& tableau`. */
            static Matrix& solve(const Matrix& tableau, Matrix& solution) {
                const size_t last = tableau._cols - 1;

                for (size_t d = 0; d < solution._cols - 1; d++) {
                    const float val = tableau(d,d);
                    const int32_t pivot_row = is_pivot(tableau, d);
                    if (pivot_row != -1) {
                        solution(0, d) = tableau(pivot_row, last);
                    } else {
                        solution(0, d) = 0.f;
                    }
                }

                solution(0, solution._cols - 1) = tableau(tableau._rows - 1, tableau._cols - 1);
                return solution;
            }
            
        public:
            /** Solves the linear programming problem for the set of free variables that maximize
             * the objective function. The first n - 1 indices correspond to the free variables,
             * and the nth index corresponds to the 'cost'. */
            Matrix&& maximize() {
                int32_t pivot = find_pivot_column(this->_tableau);
                Matrix local_solution(_solution._rows, _solution._cols, 0.f);

                while (pivot != -1) {
                    const int32_t target_row = find_target_row(this->_tableau, static_cast<size_t>(pivot));
                    solve(pivot_at(this->_tableau, pivot, target_row), local_solution);

                    // Better solution found, so copy it over.
                    if (local_solution(0, local_solution._cols - 1) >= this->_solution(0, local_solution._cols - 1)) {
                        for (size_t j = 0; j < local_solution._cols; j++) {
                            this->_solution(0, j) = local_solution(0, j);
                        }
                    }

                    pivot = find_pivot_column(this->_tableau);
                }

                return std::move(_solution);
            }
    };
};

int main() {
    constexpr size_t ROWS = 4;
    constexpr size_t COLS = 8;

    std::cout << "Hello world" << std::endl;

    // Input
    int input[ROWS][COLS] = {
        { 1,  1,  0, 1, 0, 0, 0, 3},
        { 0,  1,  1, 0, 1, 0, 0, 4},
        { 1,  0,  1, 0, 0, 1, 0, 5},
        {-1, -2, -1, 0, 0, 0, 1, 0}
    };

    std::cout << "Populating matrix" << std::endl;

    // Construct & populate matrix
    Matrix m(ROWS, COLS, 0);
    for (size_t i = 0; i < ROWS; i++) {
        for (size_t j = 0; j < COLS; j++) {
            m(i,j) = static_cast<float>(input[i][j]);
        }
    }

    LP::Simplex lp(std::move(m), 3);
    Matrix ans = lp.maximize();

    std::cout << "Solution = [" << Matrix::print(ans) << "]" << std::endl; 

    return 0;
}