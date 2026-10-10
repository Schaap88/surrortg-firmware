#ifndef MAX17043_CONVERSION_H
#define MAX17043_CONVERSION_H

#include <stdint.h>

inline float max17043_voltage_from_raw(uint16_t raw) {
    return float(raw >> 4) * 0.00125f;
}

inline float max17043_soc_from_raw(uint16_t raw) {
    return float(raw >> 8) + float(raw & 0xFF) / 256.0f;
}

#endif  // MAX17043_CONVERSION_H
