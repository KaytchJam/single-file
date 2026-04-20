#include <iostream>
#include <array>
#include <sstream>

template <size_t Degree>
class Polynomial {
private:
    std::array<float, Degree + 1> coefficients;
    
    /** Helper function. */
    float pow_local(float x, int power) const {
        float acc = 1.f;
        for (int i = 0; i < power; i++) {
            acc *= x;
        }
        return acc;
    }

public:
    Polynomial(const std::array<float, Degree + 1>& coef) : coefficients(coef) {}
    
    /** Return the coefficient `c` corresponding to size_t `degree`.
     * 
     * For example: Given `Polynomial<2> p({3, 2, 1})` corresponding 
     * to 3x^2 + 2x + 1, `p.coef_at_degree(0)` = 1, `p.coef_at_degree(1)` = 2, and 
     * `p.coef_at_degree(2)` = 3. */
    float coef_at_degree(size_t degree) const {
        return coefficients[Degree - degree];
    }

    /** Return the nth coefficient of the polynomial, starting from
     * the coefficient of highest degree to that of degree 0.
     * 
     * For example: Given `Polynomial<2> p({3, 2, 1})` corresponding
     * to 3x^2 + 2x + 1, `p.coef_at_n(0)` = 3, `p_coef_at_n(1)` = 2, and
     * `p.coef_at_n(2)` = 1.
     */
    float coef_at_n(size_t n) const {
        return coefficients[n];
    }
    
    /** Compute the polynomial at 'x' */
    float operator()(float x) const {
        float acc = 0.f;
        for (size_t i = 0; i <= Degree; i++) {
            acc = acc * x + coefficients[i];
        }
        return acc;
    }
    
    /** It prints the coefficients. What else can I say. */
    void print_coefficients() {
        size_t degree = Degree;
        for (float f : coefficients) {
            std::cout << "Degree = " << degree << ", coefficient = " << f << std::endl;
            degree -= 1;
        }
    }

    std::array<float, Degree + 1>& coefs() {
        return coefficients;
    }
    
    /** Sum two Polynomials together. The resulting Polynomial's degree is the maximum of the two. */
    template <size_t OtherDegree>
    Polynomial<(Degree > OtherDegree ? Degree : OtherDegree)> operator+(const Polynomial<OtherDegree>& other) {
        constexpr size_t MaxDegree = (Degree > OtherDegree ? Degree : OtherDegree);
        std::array<float, MaxDegree + 1> coefs{};

        for (size_t i = 0; i <= MaxDegree; i++) {
            float a = (i <= Degree) ? coef_at_degree(i) : 0.f;
            float b = (i <= OtherDegree) ? other.coef_at_degree(i) : 0.f;
            coefs[MaxDegree - i] = a + b;
        }
        
        return Polynomial<MaxDegree>(coefs);
    }

    std::string to_string() const {
        std::stringstream ss;
        for (size_t i = 0; i < Degree + 1; i++) {
            ss << coefficients[i];
            if ((Degree - i) != 0) {
                ss << "x^" << (Degree - i) << " + ";
            }
        }
        return ss.str();
    }

    std::string to_string(const float v) const {
        std::stringstream ss;
        for (size_t i = 0; i < Degree + 1; i++) {
            ss << coefficients[i];
            if ((Degree - i) != 0) {
                ss << "(" << v << ")^" << (Degree - i) << " + ";
            }
        }
        return ss.str();
    }
};

int main() {
    Polynomial<3> f({3, 2, 1, 5});
    Polynomial<2> g({2, 5, 1});
    Polynomial<3> h = f + g;

    float x;
    std::cout << "Type in some real-valued number: ";
    std::cin >> x;

    const float f_result = f(x);
    const float g_result = g(x);
    const float h_result = h(x);

    std::cout << "f(" << x << "): " << f.to_string(x) << " = " << f_result << std::endl;
    std::cout << "g(" << x << "): " << g.to_string(x) << " = " << g_result << std::endl;
    std::cout << "h(" << x << "): " << h.to_string(x) << " = " << h_result << std::endl;
    std::cout << "f(" << x << ") + g(" << x << "): " << (f + g).to_string(x) << " = " << f_result + g_result << std::endl;
    
    return 0;
}