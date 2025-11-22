#include <iostream>
#include <vector>
#include <cmath>
#include <array>

/** index based lerp */
float index_lerp(const std::vector<float>& v, const float x) {
    const float alpha = std::floor(x) + 1.f - x;
    const int index = (int) floor(x);
    return alpha * v[index] + (1.f - alpha) * v[std::min((int) v.size() - 1, index + 1)];
}

/** clamp x between low and high */
float clamp(float low, float x, float high) {
    return std::max(low, std::min(high, x));
}

/** 
 * Callable class that creates a continuous line that passes near
 * values in the input std::vector<float> 'v'. 
 */
template <int PARTS>
class WindowedLerp {
    std::array<float, PARTS + 1> nrange;
public:
    WindowedLerp() : nrange{} {
        const float high = 1.f;
        const float low = -1.f;
        
        const float fparts = static_cast<float>(PARTS);
        for (int i = 0; i <= PARTS; i++) {
            float part = (i / fparts) * (high - low) + low;
            this->nrange[i] = part;
        }
    }
    
    ~WindowedLerp() {}
    
    float operator()(const std::vector<float>& v, const float x) {
        const float coef = 1.f / (PARTS + 1);
        const float N = (float) v.size() - 1;
        float acc = 0.f;
        
        for (float part : this->nrange) {
            acc += index_lerp(v, clamp(0.f, x + part, N));
        }
        
        return coef * acc;
    }
};

int main() {
    std::vector<float> v = {10.f, 5.f, 15.f, 20.f, 13.f};
    WindowedLerp<20> cv;
    
    // the result? a continuous or "smooth" interpolation function
    // defined for all x. outside of the interval { 0 <= i <= v.size() },
    // cv(i) = v[0] if i < 0 and v[v.size() - 1] if i > v.size() - 1.
    const float dt = 0.5f;
    float x = 0.f;
    while (x <= v.size()) {
        std::cout << "(" << x << ", " << cv(v, x) << ")" << std::endl;
        x += dt;
    }
    
    return 0;
}