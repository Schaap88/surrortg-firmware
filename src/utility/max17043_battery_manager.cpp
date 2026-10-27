#include <Arduino.h>
#include <cstring>
#include "max17043_battery_manager.h"

namespace {
// Keep the historical API/file names for consumers. Rev A fits a MAX17048,
// whose VCELL, SOC and VERSION registers are compatible with this driver.
constexpr uint8_t MAX17048_ADDRESS = 0x36;
constexpr uint8_t MAX17048_VCELL_REGISTER = 0x02;
constexpr uint8_t MAX17048_SOC_REGISTER = 0x04;
constexpr uint8_t MAX17048_VERSION_REGISTER = 0x08;
constexpr unsigned long POLL_INTERVAL_MS = 10000;
constexpr unsigned long CACHE_MAX_AGE_MS = 30000;
constexpr uint8_t READ_ATTEMPTS = 2;

TwoWire* max17043_wire = nullptr;
static bool max17043_available = false;
bool cached_status_valid = false;
bool zero_soc_warning_reported = false;
bool communication_fault_reported = false;
unsigned long cached_status_at_ms = 0;
unsigned long last_poll_at_ms = 0;
BatteryStatus cached_status = {0.0f, 0.0f};

bool read_register16(uint8_t register_address, uint16_t* value) {
    if (!max17043_wire || !value) {
        return false;
    }

    for (uint8_t attempt = 0; attempt < READ_ATTEMPTS; ++attempt) {
        max17043_wire->beginTransmission(MAX17048_ADDRESS);
        if (max17043_wire->write(register_address) != 1) {
            max17043_wire->endTransmission();
            continue;
        }
        if (max17043_wire->endTransmission(false) != 0) {
            continue;
        }

        const size_t received =
                max17043_wire->requestFrom(MAX17048_ADDRESS, uint8_t(2));
        if (received != 2 || max17043_wire->available() < 2) {
            while (max17043_wire->available()) {
                max17043_wire->read();
            }
            continue;
        }

        const uint8_t msb = max17043_wire->read();
        const uint8_t lsb = max17043_wire->read();
        *value = (uint16_t(msb) << 8) | lsb;
        return true;
    }

    return false;
}

bool refresh_status(unsigned long now_ms) {
    uint16_t voltage_raw = 0;
    uint16_t soc_raw = 0;
    if (!read_register16(MAX17048_VCELL_REGISTER, &voltage_raw) ||
        !read_register16(MAX17048_SOC_REGISTER, &soc_raw)) {
        if (!communication_fault_reported) {
            Serial.println(
                    "MAX17048 read failed; retaining recent cached telemetry");
            communication_fault_reported = true;
        }
        max17043_available = false;
        return false;
    }

    BatteryStatus candidate = {
        max17043_voltage_from_raw(voltage_raw),
        max17043_soc_from_raw(soc_raw),
    };
    if (candidate.voltage <= 0.0f || candidate.voltage > 5.12f ||
        candidate.soc < 0.0f || candidate.soc > 125.0f) {
        if (!communication_fault_reported) {
            Serial.println(
                    "MAX17048 returned invalid telemetry; retaining cache");
            communication_fault_reported = true;
        }
        max17043_available = false;
        return false;
    }

    cached_status = candidate;
    cached_status_at_ms = now_ms;
    cached_status_valid = true;
    max17043_available = true;
    if (communication_fault_reported) {
        Serial.println("MAX17048 communication recovered");
        communication_fault_reported = false;
    }

    if (soc_raw == 0 && candidate.voltage >= 3.0f) {
        if (!zero_soc_warning_reported) {
            Serial.println(
                    "MAX17048 reports raw SoC 0x0000 with valid voltage");
            zero_soc_warning_reported = true;
        }
    } else {
        zero_soc_warning_reported = false;
    }
    return true;
}
}  // namespace

bool init_max17043(TwoWire& wire) {
    max17043_wire = &wire;
    cached_status_valid = false;
    zero_soc_warning_reported = false;
    communication_fault_reported = false;
    last_poll_at_ms = millis();

    uint16_t version = 0;
    max17043_available =
            read_register16(MAX17048_VERSION_REGISTER, &version);
    if (max17043_available && refresh_status(millis())) {
        Serial.println("MAX17048 initialized successfully");
        return true;
    }

    Serial.println("Failed to initialize MAX17048");
    communication_fault_reported = true;
    return false;
}

void poll_max17043(unsigned long now_ms) {
    if (now_ms - last_poll_at_ms < POLL_INTERVAL_MS) {
        return;
    }
    last_poll_at_ms = now_ms;

    if (!max17043_available) {
        uint16_t version = 0;
        max17043_available =
                read_register16(MAX17048_VERSION_REGISTER, &version);
    }
    if (max17043_available) {
        refresh_status(now_ms);
    }
}

bool read_battery_status(BatteryStatus* status) {
    if (!status || !cached_status_valid ||
        millis() - cached_status_at_ms > CACHE_MAX_AGE_MS) {
        return false;
    }
    *status = cached_status;
    return true;
}

Max17048Telemetry read_max17048_telemetry(unsigned long now_ms) {
    Max17048Telemetry result = {cached_status, 0, false};
    if (!cached_status_valid) return result;
    result.age_ms = now_ms - cached_status_at_ms;
    result.valid = result.age_ms <= CACHE_MAX_AGE_MS;
    return result;
}
