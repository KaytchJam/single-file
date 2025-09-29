#include <vector>
#include <iostream>
#include <array>
#include <limits>

/** A point. You get the point. */
template <size_t D>
struct Point {
private:
	std::array<float,D> buffer;	
public:
	Point() : buffer() {}

	template <size_t S>
	static Point<S> from(std::array<float,S>&& data) {
		return Point<S>{
			data
		};
	}

	template <size_t S>
	static Point<S> from(std::array<float,S> data) {
		return Point<S> {
			std::move(data)
		};
	}

		
	Point(std::initializer_list<float> init) {
		if (init.size() != D) {
			throw std::invalid_argument("Incorrect number of elements");
		}
		
		std::copy(init.begin(), init.end(), this->buffer.begin());	
	}

	/** Initialize a Point of zeroes */
	template <size_t S>
	static Point<S> zeroes() {
		return Point<S> {
			{}
		};
	}

	template <size_t S>
	static Point<S> ones() {
		std::array<float,S> A;
		A.fill(1);
		return Point<S> {
			std::move(A)		
		};
	}

	template <size_t S>
	static Point<S> Ns(float f) {
		std::array<float,S> A;
		A.fill(f);
		return Point<S> {
			std::move(A)
		};
	}

	float& operator[](int index) {
		return this->buffer[index];
	}

	const float& operator[](int index) const {
		return this->buffer[index];
	}

	friend Point<D> operator*(const float scalar, const Point<D>& p) {
		Point<D> output = {};
		for (int i = 0; i < D; i++) {
			output[i] = scalar * p[i];
		}
		return output;
	}

	friend Point<D> operator*(const Point<D>& p, const float scalar) {
		return scalar * p;
	}

	friend Point<D> operator+(const Point<D>& p, const Point<D>& q) {
		Point<D> output = {};
		for (int i = 0; i < D; i++) {
			output[i] = p[i] + q[i];
		}
		return output;
	}

	friend Point<D> operator-(const Point<D>& p, const Point<D>& q) {
		Point<D> output = {};
		for (int i = 0; i < D; i++) {
			output[i] = p[i] - q[i];
		}
		return output;
	}

	friend std::ostream& operator<<(std::ostream& os, const Point<D>& p) {
		os << "[";
		for (int i = 0; i < D; i++) {
			os << p[i] << ", ";
		}
		os << "]";
		return os;
	}
};

/** A cute lagrange interpolation model implementation */
template<typename NumericType>
class Lagrange {
private:
    struct LagrangeBasis {
        float x;
        NumericType control_point; // needs to have addition & scalar mult

        void print() const {
            std::cout << "(" << this->x << ", " << this->control_point << ")";
        }

        std::pair<float, NumericType> as_pair() const {
            return {this->x, this->control_point};
        }

        std::pair<float, NumericType*> as_ptr_pair() const {
            return {this->x, &this->control_point};
        }
    };

    float m_x_min;
    float m_x_max;
    std::vector<LagrangeBasis> m_basis_pairs;
public:
    Lagrange(const std::vector<float>& x_values, const std::vector<NumericType>& control_pts) {
        const int num_values = (int) x_values.size();
        this->m_basis_pairs = std::vector<LagrangeBasis>(num_values);
        this->m_x_min = std::numeric_limits<float>::max();
        this->m_x_max = std::numeric_limits<float>::min();

        for (int i = 0; i < num_values; i++) {
            LagrangeBasis& basis_pair = this->m_basis_pairs[i];
            basis_pair.x = x_values[i];

            this->m_x_max = std::max(this->m_x_max, basis_pair.x);
            this->m_x_min = std::min(this->m_x_min, basis_pair.x);

            float denom_product = 1.f;
            for (int j = 0; j < num_values; j++) {
                const bool neq_gate = j != i;
                denom_product *= (basis_pair.x - x_values[j]) * neq_gate + !neq_gate;
            }

            basis_pair.control_point = (1.f/denom_product) * control_pts[i];
        }
    }

    template<typename Iter>
    Lagrange(const Iter& start_x, const Iter& end_x, const Iter& start_y, const Iter& end_y) {
        this->m_x_max = std::numeric_limits<float>::max();
        this->m_x_min = std::numeric_limits<float>::min();

        // We assume both of these iterators travel some buffer of the same length
        Iter y_iter = start_y;
        for (Iter x_iter = start_x; x_iter != end_x; x_iter++) {
            LagrangeBasis basis_pair;
            basis_pair.x = (float) *x_iter;
         
            float denom_product = 1;
            for (Iter x_dupe = start_x; x_dupe != end_x; x_dupe++) {
                const bool neq_gate = x_dupe != x_iter;
                denom_product *= (basis_pair.x - (*x_dupe)) * neq_gate + !neq_gate;
            }

            basis_pair.control_point = (*y_iter) * (1.f / denom_product);
            this->m_basis_pairs.push_back(basis_pair);
            y_iter = std::next(y_iter);
        }
    }

    Lagrange(Lagrange<NumericType>&& other) {
        this->m_x_max = other.m_x_max;
        this->m_x_min = other.m_x_min;
        this->m_basis_pairs = std::move(other.m_basis_pairs);
    }

    // Returns the minimum and maximum float this Lagrange instance
    // was defined over (has an associated control point with)
    // as an std::pair<float, float> = {min, max}
    std::pair<float, float> domain() {
        return {this->m_x_min, this->m_x_max};
    }

    // Compute 
    NumericType compute(float x) const {
        const int num_values = (int) this->m_basis_pairs.size();
        NumericType output = 0 * this->m_basis_pairs[0].control_point;

        for (int i = 0; i < num_values; i++) {
            const LagrangeBasis& cur_basis = this->m_basis_pairs[i];
            float product = 1;

            for (int j = 0; j < num_values; j++) {
                const LagrangeBasis& other_basis = this->m_basis_pairs[j];
                const bool neq_gate = cur_basis.x != other_basis.x;
                product *= (x - other_basis.x) * neq_gate + !neq_gate;
            }

            output = output + product * cur_basis.control_point;
        }

        return output;
    }

    // Compute an entire set
    std::vector<NumericType> compute_all(const std::vector<float>& x_set) const {
        std::vector<NumericType> results(x_set.size());
        for (int i = 0; i < x_set.size(); i++) {
            results[i] = this->compute(x_set[i]);
        }

        return results;
    }


    // Computes num_points number of points in the linear space
    std::vector<NumericType> compute_range(float x_min, float x_max, int num_points) {
        const float partition = (x_max - x_min) / (num_points - 1);
        float current = x_min;
        std::vector<NumericType> range_computations(num_points);

        for (int i = 0; i < num_points; i++) {
            range_computations[i] = this->compute(current);
            current += partition;
        }

        range_computations[num_points - 1] = this->compute(x_max);
        return range_computations;
    }

    static void epsilon_check(float epsilon) {
        if (epsilon <= 0) {
            std::cout << "Parameter float NumericType::compute_derivative::epsilon must be greater than 0";
            std::exit(EXIT_FAILURE);
        }
    }

    static float calculate_direction(float x1, float x2) {
        float sum = x2 - x1;
        return sum / abs(sum);
    }

    // Approximates the derivative at some point x along this Lagrangian curve
    NumericType approximate_derivative(float x, float epsilon=0.001f) {
        Lagrange::epsilon_check(epsilon);
        return (this->compute(x + epsilon) - this->compute(x)) / epsilon;
    }

    // Returns the slope between two points along this Lagrangian curve given an x1 and x2
    NumericType compute_slope(float x1, float x2) {
        return (this->compute(x2) - this->compute(x1)) / (x2 - x1);
    }

    // Computes num_points number of points in the linear space [this->range().first, this->range().second]
    std::vector<NumericType> compute_range(int num_points) {
        return this->compute_range(this->m_x_min, this->m_x_max, num_points);
    }
};

int main() {
	std::vector<float> xvals = {1.f, 2.f, 3.f, 4.f, 5.f};
	std::vector<Point<2>> ps = {
		{1.f, 5.f},
		{2.f, 2.f},
		{4.f, 1.f},
		{6.f, 2.f},
		{3.f, 5.f}	
	};

	Lagrange<Point<2>> lag(xvals, ps);
	Point<2> result = lag.compute(1.5f);
	std::cout << "The result is: " << result << "." << std::endl;

	return EXIT_SUCCESS;
}
