#include <iostream>
#include <array>

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
    Polynomial(std::array<float, Degree + 1>&& coef) : coefficients(coef) {}
    
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
        for (int i = 0; i < Degree + 1; i++) {
            acc += coefficients[i] * pow_local(x, Degree - i);
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
    
    /** Sum two Polynomials together. The resulting Polynomial's degree is the maximum of the two. */
    template <size_t OtherDegree>
    Polynomial<(Degree > OtherDegree ? Degree : OtherDegree)> operator+(const Polynomial<OtherDegree>& other) {
        constexpr size_t MaxDegree = (Degree > OtherDegree ? Degree : OtherDegree);
        constexpr size_t MinDegree = (Degree < OtherDegree ? Degree : OtherDegree);
        
        std::array<float, MaxDegree + 1> coefs{};
        const Polynomial<MaxDegree>* ref = (MaxDegree == Degree) ? (Polynomial<MaxDegree>*) this : (Polynomial<MaxDegree>*) &other;

        for (int i = (int) MaxDegree; i > MinDegree; i -= 1) {
            coefs[MaxDegree - i] = ref->coef_at_degree(i);
        }
        
        for (int i = (int) MinDegree; i >= 0; i -= 1) {
            coefs[MaxDegree - i] = coef_at_degree(i) + other.coef_at_degree(i);
        }
        
        return Polynomial<MaxDegree>(std::move(coefs));
    }
};

int main() {
    Polynomial<3> f({3, 2, 1, 5});
    Polynomial<2> g({2, 5, 1});
    Polynomial<3> fg = f + g;

    const float f_result = f(3);
    const float g_result = g(3);
    const float fg_result = fg(3);

    std::cout << "f(3) result = " << f_result << std::endl;
    std::cout << "g(3) result = " << g_result << std::endl;
    std::cout << "fg(3) result = " << fg_result << std::endl;
    std::cout << "f(3) + g(3) result = " << f_result + g_result << std::endl;
    
    return 0;
}