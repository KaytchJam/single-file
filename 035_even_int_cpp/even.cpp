#include <iostream>
#include <type_traits>
#include <optional>
#include <cstdint>

template <typename IntType>
struct even_int_t {
    private:
        static_assert(std::is_integral_v<IntType>, "Type 'IntType' is not integral.");
        
        IntType num;
        explicit constexpr even_int_t(IntType i) : num(i) {}
    
    public:
        even_int_t() : num(0) {}
    
        static constexpr std::optional<even_int_t> create(IntType i) {
            if ((i & 0x1) == 1) {
                return std::nullopt;
            }
            return even_int_t(i);
        }
        
        static even_int_t create_unchecked(IntType i) {
            return even_int_t(i);
        }
        
        even_int_t add_even(IntType i) {
            return even_int_t( num + (i & ((size_t) 0 - !((i & 0x1) == 1))) );
        }
        
        even_int_t add_2k(IntType k) {
            return even_int_t(num + 2*k);
        }

        even_int_t operator+(even_int_t other) const {
            return even_int_t(num + other.num);
        }
        
        IntType value() { return num; }

        constexpr explicit operator IntType() const { return num; }
};

using even_i64 = even_int_t<int64_t>;
using even_i32 = even_int_t<int32_t>;
using even_i16 = even_int_t<int16_t>;
using even_i8 = even_int_t<int8_t>;

using even_u64 = even_int_t<uint64_t>;
using even_u32 = even_int_t<uint32_t>;
using even_u16 = even_int_t<uint16_t>;
using even_u8 = even_int_t<uint8_t>;

int main() {
    even_i32 i = even_i32::create_unchecked(2);
    even_i32 j = i.add_even(1);
    even_i32 k = i.add_even(2);
    even_i32 l = i.add_2k(2);
    
    std::cout << j.value() << std::endl;
    std::cout << k.value() << std::endl;
    std::cout << l.value() << std::endl;
    return 0;
}