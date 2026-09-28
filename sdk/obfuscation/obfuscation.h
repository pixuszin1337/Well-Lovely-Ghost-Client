#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>

namespace obf {

    template<uint32_t Seed>
    struct prng {
        static constexpr uint32_t value = Seed * 0x41C64E6D + 0x3039;
        static constexpr uint32_t next = value * 0x41C64E6D + 0x3039;
    };

    template<typename T, T Value, uint32_t Key>
    struct constant {
        static constexpr T encrypted = static_cast<T>(
            static_cast<uint64_t>(Value) ^ static_cast<uint64_t>(Key));

        __forceinline static T get() {
            volatile T enc = encrypted;
            volatile uint32_t k = Key;
            return static_cast<T>(
                static_cast<uint64_t>(enc) ^ static_cast<uint64_t>(k));
        }
    };

    template<uint32_t Key>
    struct float_constant {
        uint32_t encrypted_bits;

        float_constant(float value) {
            uint32_t bits;
            std::memcpy(&bits, &value, sizeof(bits));
            encrypted_bits = bits ^ Key;
        }

        __forceinline float get() const {
            volatile uint32_t enc = encrypted_bits;
            volatile uint32_t k = Key;
            uint32_t raw = enc ^ k;
            float result;
            std::memcpy(&result, &raw, sizeof(result));
            return result;
        }
    };

}

#define OBF_INT(val) (obf::constant<int, (val), __LINE__ * 0x9E3779B9 + 0xDEAD>::get())
#define OBF_UINT(val) (obf::constant<uint32_t, (val), __LINE__ * 0x9E3779B9 + 0xBEEF>::get())
#define OBF_FLOAT(val) ([]{ \
    static const obf::float_constant<__LINE__ * 0x9E3779B9 + 0xCAFE> c(val); \
    return c.get(); \
}())
