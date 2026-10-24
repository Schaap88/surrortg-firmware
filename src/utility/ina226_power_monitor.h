#ifndef INA226_POWER_MONITOR_H
#define INA226_POWER_MONITOR_H

#include <Wire.h>
#include <stdint.h>

struct Ina226Telemetry {
    int32_t current_ua;
    uint32_t age_ms;
    bool valid;
};

constexpr uint8_t INA226_REV_A_ADDRESS = 0x41;
constexpr uint16_t INA226_REV_A_CONFIGURATION = 0x4527;
constexpr uint16_t INA226_REV_A_CALIBRATION = 0x1400;

inline int32_t ina226_current_ua_from_raw(uint16_t raw) {
    return static_cast<int32_t>(static_cast<int16_t>(raw)) * 100;
}

bool init_ina226(TwoWire& wire);
void poll_ina226(unsigned long now_ms);
Ina226Telemetry read_ina226_telemetry(unsigned long now_ms);

#endif  // INA226_POWER_MONITOR_H
