#include <array>
#include <cassert>
#include <iostream>
#include <cmath>

/** Base class representing vectors and vector expressions. Utilizes CRTP. */
template <typename Derived>
class VecExpression {
public:
    double operator[](size_t i) const {
        return static_cast<const Derived&>(*this)[i];
    }

    size_t size() const {
        return static_cast<const Derived&>(*this).size();
    }
};

/** Representation of a simple 3D vector */
class Vec3D : public VecExpression<Vec3D> {
private:
    std::array<double,3> data;
public:

    Vec3D(std::initializer_list<double> init) {
        assert(init.size() == 3);
        std::initializer_list<double>::iterator it = init.begin();
        for (int i = 0; i < 3; i++) {
            data[i] = *it;
            ++it;
        }
    }

    Vec3D() : data({0,0,0}) {}
    Vec3D(double a, double b, double c) : data({a,b,c}) {}

    double operator[](size_t i) const {
        return data[i];
    }

    double& operator[](size_t i) {
        return data[i];
    }

    size_t size() const {
        return data.size();
    }

    template <typename Derived>
    Vec3D(const VecExpression<Derived>& ve) {
        for (size_t i = 0; i < ve.size(); i++) {
            data[i] = ve[i];
        }
    }
};

/** Representation of the sum of 2 vectors */
template <typename Exp1, typename Exp2>
class SumVec3D : public VecExpression<SumVec3D<Exp1,Exp2>> {
private:
    const Exp1& v;
    const Exp2& w;
public:

    SumVec3D(const Exp1& v, const Exp2& w) : v(v), w(w) {
        assert(v.size() == w.size());
    }

    double operator[](size_t i) const {
        return v[i] + w[i];
    }

    size_t size() const {
        return v.size();
    }
};

/** Representation of the sum of 2 vectors */
template <typename Exp1, typename Exp2>
class SubVec3D : public VecExpression<SubVec3D<Exp1,Exp2>> {
private:
    const Exp1& v;
    const Exp2& w;
public:

    SubVec3D(const Exp1& v, const Exp2& w) : v(v), w(w) {
        assert(v.size() == w.size());
    }

    double operator[](size_t i) const {
        return v[i] - w[i];
    }

    size_t size() const {
        return v.size();
    }
};

/** Representation of multiplying a vector with a scalar */
template <typename Exp1>
class ScaleVec3D : public VecExpression<ScaleVec3D<Exp1>> {
private:
    const Exp1& v;
    const double scalar;
public:
    ScaleVec3D(const Exp1& v, double scalar) : v(v), scalar(scalar) {}

    double operator[](size_t i) const {
        return v[i] * scalar;
    }

    size_t size() const {
        return v.size();
    }
};

/** LAZY EXPRESSIONS */

template <typename E1, typename E2>
SumVec3D<E1,E2> operator+(const VecExpression<E1>& v, const VecExpression<E2>& w) {
    return SumVec3D<E1,E2>(static_cast<const E1&>(v), static_cast<const E2&>(w));
}

template <typename E1, typename E2>
SubVec3D<E1,E2> operator-(const VecExpression<E1>& v, const VecExpression<E2>& w) {
    return SubVec3D<E1,E2>(static_cast<const E1&>(v), static_cast<const E2&>(w));
}

template <typename E1>
ScaleVec3D<E1> operator*(double scalar, const VecExpression<E1>& v) {
    return ScaleVec3D<E1>(static_cast<const E1&>(v), scalar);
}

template <typename E1>
ScaleVec3D<E1> operator*(const VecExpression<E1>& v, double scalar) {
    return ScaleVec3D<E1>(static_cast<const E1&>(v), scalar);
}

template <typename E1>
ScaleVec3D<E1> operator/(const VecExpression<E1>& v, double scalar) {
    return ScaleVec3D<E1>(static_cast<const E1&>(v), 1.0 / scalar);
}

/** EAGER EXPRESSIONS ("ACTIVATE" ALL THE LAZY EXPRESSIONS FOR A COMPUTATION) */

template <typename E1, typename E2>
double dot(const VecExpression<E1>& v, const VecExpression<E2>& w) {
    assert(v.size() == w.size());
    double acc = 0.0;
    for (int i = 0; i < v.size(); i++) {
        acc += v[i] * w[i];
    }
    return acc;
}

template <typename E1>
double magnitude(const VecExpression<E1>& v) {
    return sqrt(dot(v, v));
}

template <typename E1, typename E2>
double cosineOf(const VecExpression<E1>& v, const VecExpression<E2>& w) {
    return (dot(v, w) / (magnitude(v) * magnitude(w)));
}


/** HYBRID LAZY + EAGER */

template <typename E1>
ScaleVec3D<Vec3D> unit(const VecExpression<E1>& v) {
    Vec3D v_evaluated = v;
    const double mag = magnitude(v_evaluated);
    return v_evaluated / mag;
}

template <typename E1, typename E2>
ScaleVec3D<Vec3D> projectOnto(const VecExpression<E1>& v, const VecExpression<E2>& onto) {
    Vec3D b_unit = unit(onto);
    return dot(v, b_unit) * b_unit;
}

template <typename E1, typename E2>
Vec3D cross(const VecExpression<E1>& v, const VecExpression<E2>& w) {
    Vec3D i(1.0, 0.0, 0.0);
    Vec3D j(0.0, 1.0, 0.0);
    Vec3D k(0.0, 0.0, 1.0);

    return ((v[1] * w[2] - v[2] * v[1]) * i) 
    - ((v[0] * w[2] - v[2] * w[0]) * j)
    + ((v[0] * w[1] - v[1] * w[0]) * k);
}

/** TESTS */

struct TestResult {
    const char* test_name;
    const bool test_result;
};

TestResult threeSumTest() {
    Vec3D v(2.0, 3.0, 4.0);
    Vec3D w(1.0, 2.0, 3.0);
    Vec3D u(1.0, 1.0, 1.0);
    Vec3D res1 = 2.0 * (v + w + u); 

    SumVec3D<Vec3D,Vec3D> step1 = v + w;
    SumVec3D<SumVec3D<Vec3D,Vec3D>,Vec3D> step2 = step1 + u;
    ScaleVec3D<SumVec3D<SumVec3D<Vec3D,Vec3D>,Vec3D>> step3 = 2.0 * step2;
    Vec3D res2 = step3; // converted to Vec3D via constructor

    bool all_eq = true;
    for (int i = 0; i < res1.size(); i++) {
        all_eq = all_eq && (res1[i] == res2[i]);
    }

    all_eq = all_eq && (res1[0] == 8 && res1[1] == 12 && res1[2] == 16);
    return TestResult{ "Three Sum Test", all_eq };
}

TestResult subtractTest() {
    Vec3D v(0.0, 1.0, 0.0);
    Vec3D w(1.0, 0.0, 1.0);
    Vec3D result = (v + w) - v - w;
    bool correct = result[0] == 0 && result[1] == 0 && result[2] == 0;
    return TestResult{ "Subtract Test", correct};
}

TestResult dotProductTest() {
    Vec3D a(0.0, 1.0, 0.0);
    Vec3D b(0.0, 2.0, 0.0);
    double result1 = dot(a,b);
    
    // Testing w/ Orthogonal Vectors
    Vec3D v(1.0, 0.0, 0.0);
    double result2 = dot(v, a + b);
    
    bool correct = result1 == 2.0 && result2 == 0;
    return TestResult{ "Dot Product Test", correct };
}

TestResult unitVectorTest() {
    Vec3D v(0.0, 0.0, 5.0);
    Vec3D u = unit(v);
    bool correct = u[0] == 0 && u[1] == 0 && u[2] == 1 && magnitude(u) == 1;
    return TestResult{ "Unit Vector Test", correct };
}

TestResult crossProductTest() {
    Vec3D u(1.0, 0.0, 0.0);
    Vec3D v(0.0, 1.0, 0.0);
    Vec3D c = cross(u,v);
    bool correct = c[0] == 0 && c[1] == 0 && c[2] == 1;
    return TestResult{ "Cross Product Test", correct };
}

/** MAIN */

typedef TestResult (*TestFunc)();

int main() {
    TestFunc tests[] = { threeSumTest, subtractTest, dotProductTest, unitVectorTest, crossProductTest };
    int idx = 1;

    for ( TestFunc t : tests ) {
        TestResult outcome = t();
        std::cout << idx << ") The outcome of Test '" << outcome.test_name << "' = " << std::boolalpha << outcome.test_result << std::endl;
        idx += 1;
    }
}