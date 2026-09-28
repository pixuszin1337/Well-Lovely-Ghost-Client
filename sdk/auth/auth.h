#pragma once
#include <cstdint>
#include <cstring>

namespace auth {

    uint32_t session_key();

    inline float decode_float(float encoded, uint32_t expected_key) {
        uint32_t raw;
        memcpy(&raw, &encoded, 4);
        raw ^= expected_key;
        float result;
        memcpy(&result, &raw, 4);
        return result;
    }

    inline float bits_to_float(uint32_t bits) {
        float f;
        memcpy(&f, &bits, 4);
        return f;
    }

}
