#include <Arduino.h>

#include "ina226_power_monitor.h"

namespace {
constexpr uint8_t CONFIGURATION_REGISTER = 0x00;
constexpr uint8_t CURRENT_REGISTER = 0x04;
constexpr uint8_t CALIBRATION_REGISTER = 0x05;
constexpr unsigned long POLL_INTERVAL_MS = 1000;
constexpr unsigned long CACHE_MAX_AGE_MS = 3000;
constexpr uint8_t I2C_ATTEMPTS = 2;

TwoWire* monitor_wire = nullptr;
bool available = false;
bool cached_valid = false;
bool fault_reported = false;
int32_t cached_current_ua = 0;
unsigned long cached_at_ms = 0;
unsigned long last_poll_at_ms = 0;

bool write_register16(uint8_t reg, uint16_t value) {
    if (!monitor_wire) return false;
    for (uint8_t attempt = 0; attempt < I2C_ATTEMPTS; ++attempt) {
        monitor_wire->beginTransmission(INA226_REV_A_ADDRESS);
        if (monitor_wire->write(reg) != 1 ||
            monitor_wire->write(static_cast<uint8_t>(value >> 8)) != 1 ||
            monitor_wire->write(static_cast<uint8_t>(value)) != 1) {
            monitor_wire->endTransmission();
            continue;
        }
        if (monitor_wire->endTransmission() == 0) return true;
    }
    return false;
}

bool read_register16(uint8_t reg, uint16_t* value) {
    if (!monitor_wire || !value) return false;
    for (uint8_t attempt = 0; attempt < I2C_ATTEMPTS; ++attempt) {
        monitor_wire->beginTransmission(INA226_REV_A_ADDRESS);
        if (monitor_wire->write(reg) != 1) {
            monitor_wire->endTransmission();
            continue;
        }
        if (monitor_wire->endTransmission(false) != 0) continue;
        const size_t received =
                monitor_wire->requestFrom(INA226_REV_A_ADDRESS, uint8_t(2));
        if (received != 2 || monitor_wire->available() < 2) {
            while (monitor_wire->available()) monitor_wire->read();
            continue;
        }
        *value = (uint16_t(monitor_wire->read()) << 8) |
                 uint8_t(monitor_wire->read());
        return true;
    }
    return false;
}

bool configure() {
    if (!write_register16(CONFIGURATION_REGISTER,
                          INA226_REV_A_CONFIGURATION) ||
        !write_register16(CALIBRATION_REGISTER,
                          INA226_REV_A_CALIBRATION)) {
        return false;
    }
    uint16_t calibration = 0;
    return read_register16(CALIBRATION_REGISTER, &calibration) &&
           calibration == INA226_REV_A_CALIBRATION;
}

bool refresh(unsigned long now_ms) {
    uint16_t raw = 0;
    if (!read_register16(CURRENT_REGISTER, &raw)) {
        available = false;
        if (!fault_reported) {
            Serial.println("INA226 read failed; retaining recent cache");
            fault_reported = true;
        }
        return false;
    }
    cached_current_ua = ina226_current_ua_from_raw(raw);
    cached_at_ms = now_ms;
    cached_valid = true;
    available = true;
    if (fault_reported) Serial.println("INA226 communication recovered");
    fault_reported = false;
    return true;
}
}  // namespace

bool init_ina226(TwoWire& wire) {
    monitor_wire = &wire;
    available = false;
    cached_valid = false;
    fault_reported = false;
    last_poll_at_ms = millis();
    available = configure();
    if (available && refresh(millis())) {
        Serial.println("INA226 initialized successfully");
        return true;
    }
    Serial.println("Failed to initialize INA226");
    fault_reported = true;
    return false;
}

void poll_ina226(unsigned long now_ms) {
    if (now_ms - last_poll_at_ms < POLL_INTERVAL_MS) return;
    last_poll_at_ms = now_ms;
    if (!available) available = configure();
    if (available) refresh(now_ms);
}

Ina226Telemetry read_ina226_telemetry(unsigned long now_ms) {
    Ina226Telemetry result = {cached_current_ua, 0, false};
    if (!cached_valid) return result;
    result.age_ms = now_ms - cached_at_ms;
    result.valid = result.age_ms <= CACHE_MAX_AGE_MS;
    return result;
}
