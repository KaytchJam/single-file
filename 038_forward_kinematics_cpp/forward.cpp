#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

struct vec3 {
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    constexpr vec3() = default;
    constexpr vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    constexpr vec3(float a) :  x(a), y(a), z(z) {} 
    constexpr vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}
};

std::string to_string(const vec3& v) {
    std::stringstream ss;
    ss << "| " << v.x << " , " << v.y << " , " << v.z << " |";
    return ss.str();
}

constexpr float dot(const vec3& v, const vec3& w) {
    return v.x * w.x + v.y * w.y + v.z * w.z;
}

// Scales vector v by a scalar elementwise
constexpr vec3 operator*(const float scalar, const vec3& v) {
    return vec3{scalar * v.x, scalar * v.y, scalar * v.z};
}

// Divides vector v by a scalar elementwise
constexpr vec3 operator/(const vec3& v, const float scalar) {
    return vec3{v.x / scalar, v.y / scalar, v.z / scalar};
}

// Elementwise addition between two vectors v and w
constexpr vec3 operator+(const vec3& v, const vec3& w) {
    return vec3{v.x + w.x, v.y + w.y, v.z + w.z};
}

// Elementwise subtraction between two vectors v and w
constexpr vec3 operator-(const vec3& v, const vec3& w) {
    return vec3{v.x - w.x, v.y - w.y, v.z - w.z};
}

// Returns the length of a vector
constexpr float magnitude(const vec3& v) {
    return std::sqrtf(dot(v, v));
}

// Return the normalized form of the vector, such that `magnitude(unit(v)) = 1`
constexpr vec3 unit(const vec3& v) {
    return v / magnitude(v);
}

constexpr vec3 cross(const vec3& v, const vec3& w) {
    return vec3{
        (v.y * w.z - v.z * w.y), 
        -(v.x * w.z - v.z * w.x),
        (v.x * w.y - v.y * w.x)
    };
}

// a, b, and c are column vectors
struct mat3 {
    vec3 a;
    vec3 b;
    vec3 c;

    constexpr mat3() = default;
    constexpr mat3(const vec3& a, const vec3& b, const vec3& c) : a(a), b(b), c(c) {}
    constexpr mat3(const vec3& v) : a(v), b(v), c(v) {}
    constexpr mat3(float s) : a(s), b(s), c(s) {}
    constexpr mat3(const mat3& M) : a(M.a), b(M.b), c(M.c) {}

    mat3(float a0, float b0, float c0, float a1, float b1, float c1, float a2, float b2, float c2) 
        : a(a0, a1, a2), b(b0, b1, b2), c(c0, c1, c2) {}
};

std::string to_string(const mat3& M) {
    std::stringstream ss;
    ss << "| " << M.a.x << " , " << M.b.x << " , " << M.c.x << " |\n";
    ss << "| " << M.a.y << " , " << M.b.y << " , " << M.c.y << " |\n";
    ss << "| " << M.a.z << " , " << M.b.z << " , " << M.c.z << " |";
    return ss.str();
}

// Scales matrix m by a scalar elementwise
constexpr mat3 operator*(const float scalar, const mat3& M) {
    return mat3{scalar * M.a, scalar * M.b, scalar * M.c};
}

// Divides vector v by a scalar elementwise
constexpr mat3 operator/(const mat3& M, const float scalar) {
    return mat3{M.a / scalar, M.b / scalar, M.c / scalar};
}

// Transform a vector v by a matrix M
constexpr vec3 operator*(const mat3& M, const vec3& v) {
    return (v.x * M.a) + (v.y * M.b) + (v.z * M.c);
}

// Transform a matrix W by a matrix M
constexpr mat3 operator*(const mat3& M, const mat3& W) {
    return mat3{
        M * W.a,
        M * W.b,
        M * W.c
    };
}

constexpr mat3 operator+(const mat3& M, const mat3& W) {
    return mat3{
        W.a + M.a,
        W.b + M.b,
        W.c + M.c
    };
}

constexpr mat3 operator-(const mat3& M, const mat3& W) {
    return mat3{
        M.a - W.a,
        M.b - W.b,
        M.c - W.c
    };
}

constexpr vec3& fill(vec3& v, const float scalar) {
    v.x = scalar;
    v.y = scalar;
    v.z = scalar;
    return v;
}

constexpr mat3& fill(mat3& M, const float scalar) {
    fill(M.a, scalar);
    fill(M.b, scalar);
    fill(M.c, scalar);
    return M;
}

constexpr mat3 identity() {
    return mat3{
        vec3{ 1.f, 0.f, 0.f},
        vec3{ 0.f, 1.f, 0.f},
        vec3{ 0.f, 0.f, 1.f}
    };
}

// DET(M)
// | a.x b.x c.x |
// | a.y b.y c.y |
// | a.z b.z c.z |

constexpr float determinant(const mat3& M) {
    const float A = M.a.x * (M.b.y * M.c.z - M.c.y * M.b.z);
    const float B = M.b.x * (M.a.y * M.c.z - M.c.y * M.a.z);
    const float C = M.c.x * (M.a.y * M.b.z - M.b.y * M.a.z);
    return A - B + C;
}

mat3 inverse(const mat3& M) {
    const float det = determinant(M);
    mat3 I = {
        vec3(
            M.b.y * M.c.z - M.c.y * M.b.z, 
            -(M.a.y * M.c.z - M.c.y * M.a.z) , 
            M.a.y * M.b.z - M.b.y * M.a.z
        ),
        -1.f * vec3(
            M.b.x * M.c.z - M.c.x * M.b.z, 
            -(M.a.x * M.c.z - M.c.x * M.a.z), 
            M.a.x * M.b.z - M.b.x * M.a.z
        ),
        vec3(
            M.b.x * M.c.y - M.c.x * M.b.y, 
            -(M.a.x * M.c.y - M.c.x * M.a.y), 
            M.a.x * M.b.y - M.b.x * M.a.y
        )
    };

    return I / det;
}

// start point and end point are in world coordinates
struct Arm {
    vec3 start_point;
    vec3 end_point;
    int child_id = -1;
};

struct Transform {
    mat3 basis = identity(); // local -> parent
    mat3 to_world = identity(); // local -> world
};

struct ArmBasis {
    Transform transform;
    float theta = 0.f;
    int child_id = -1;
};

ArmBasis make_basis(const Arm& arm, ArmBasis& parent) {
    /** The parent matrix (is assumed) to transform a vector in its local coordinate system
     * into world coordinates. Therefore, the inverse does the opposite and takes a vector
     * in world coordinates and transforms it into the parent's local coordinate system */

    const mat3 world_to_parent = inverse(parent.transform.to_world);

    const vec3 s_local = world_to_parent * arm.start_point;
    const vec3 e_local = world_to_parent * arm.end_point;

    const vec3 up = unit(e_local - s_local);
    const vec3 right = vec3(up.y, -up.x, 0);

    const mat3 basis = mat3(
        right.x, up.x, s_local.x,
        right.y, up.y, s_local.y,
        0.f,      0.f,       1.f
    );

    Transform transform = Transform{
        .basis = basis,
        .to_world = parent.transform.to_world * basis
    };

    return ArmBasis{transform, 0.f};
}

std::vector<Arm> get_arms() {
    Arm arm1 = { vec3(0.f, 0.f, 1.f), vec3(0.f, 1.f, 1.f), 1 };
    Arm arm2 = { arm1.end_point, vec3(1.f, 1.f, 1.f), 2 };
    Arm arm3 = { arm2.end_point, vec3(1.f, 2.f, 1.f), -1 };

    std::vector<Arm> arms = { arm1, arm2, arm3 };
    return arms;
}

std::vector<ArmBasis> make_basis_arms(int root, std::vector<Arm>& arms) {
    if (root < 0 || root >= arms.size()) {
        return std::vector<ArmBasis>(); // empty vector
    }

    std::vector<ArmBasis> bases;
    bases.reserve(arms.size());

    ArmBasis world_basis = { .transform = Transform{}, .theta = 0.f, .child_id = -1 };
    int last_basis_id = -1;

    while (root != -1) {
        Arm& current = arms[root];
        ArmBasis* const prev_basis = last_basis_id >= 0 ? (&bases[last_basis_id]) : (&world_basis);

        bases.push_back(make_basis(current, *prev_basis));

        prev_basis->child_id = root;
        last_basis_id = root;
        root = current.child_id;
    }

    return bases;
}

vec3 transform(vec3 local_point, int root, std::vector<ArmBasis>& bases) {
    ArmBasis* last = nullptr;

    while (root != -1) {
        last = &bases[root];
        root = last->child_id;
    }

    return last != nullptr ? last->transform.to_world * local_point : vec3();
}

int main() {
    std::vector<Arm> arms = get_arms();
    std::vector<ArmBasis> bases = make_basis_arms(0,arms);
    
    vec3 local_point = vec3(0.f, 1.f, 1.f);
    vec3 world_point = transform(local_point, 0, bases);

    std::cout << "LOCAL = " << to_string(local_point) << std::endl;
    std::cout << "AB3 -> WORLD = " << to_string(world_point) << std::endl;

    return EXIT_SUCCESS;
}