#include <cmath>
#include <limits.h>

#include "power_status.h"

namespace {
void put_u16(uint8_t* out, uint16_t value) {
    out[0] = static_cast<uint8_t>(value >> 8);
    out[1] = static_cast<uint8_t>(value);
}

void put_i16(uint8_t* out, int16_t value) {
    put_u16(out, static_cast<uint16_t>(value));
}

void put_i32(uint8_t* out, int32_t value) {
    const uint32_t bits = static_cast<uint32_t>(value);
    out[0] = static_cast<uint8_t>(bits >> 24);
    out[1] = static_cast<uint8_t>(bits >> 16);
    out[2] = static_cast<uint8_t>(bits >> 8);
    out[3] = static_cast<uint8_t>(bits);
}

uint16_t age_deciseconds(bool valid, uint32_t age_ms) {
    if (!valid) return SRTG_POWER_AGE_UNAVAILABLE;
    const uint32_t value = age_ms / 100;
    return value >= SRTG_POWER_AGE_UNAVAILABLE
                   ? SRTG_POWER_AGE_UNAVAILABLE - 1
                   : static_cast<uint16_t>(value);
}
}  // namespace

void serialize_power_status(uint8_t command_id,
                            const Max17048Telemetry& fuel,
                            const Ina226Telemetry& current,
                            const DockPowerTelemetry& dock,
                            uint8_t response[SRTG_POWER_STATUS_RESPONSE_SIZE]) {
    const bool voltage_valid =
            fuel.valid && std::isfinite(fuel.status.voltage) &&
            fuel.status.voltage >= 0.0f && fuel.status.voltage <= 65.535f;
    const bool soc_valid = fuel.valid && std::isfinite(fuel.status.soc) &&
                           fuel.status.soc >= 0.0f &&
                           fuel.status.soc <= 100.0f;
    const int32_t current_ma = current.current_ua / 1000;
    const bool current_valid = current.valid && current_ma >= INT16_MIN &&
                               current_ma <= INT16_MAX;
    const bool power_valid = voltage_valid && current_valid;

    uint8_t flags = 0;
    if (voltage_valid) flags |= POWER_VOLTAGE_VALID;
    if (soc_valid) flags |= POWER_SOC_VALID;
    if (current_valid) flags |= POWER_CURRENT_VALID;
    if (power_valid) flags |= POWER_POWER_VALID;
    if (dock.valid) flags |= POWER_DOCK_VALID;

    response[0] = command_id;
    response[1] = SRTG_POWER_STATUS_VERSION;
    response[2] = flags;
    put_u16(&response[3], voltage_valid
                                     ? static_cast<uint16_t>(
                                               std::lround(fuel.status.voltage *
                                                           1000.0f))
                                     : 0);
    put_u16(&response[5], soc_valid
                                     ? static_cast<uint16_t>(
                                               std::lround(fuel.status.soc *
                                                           100.0f))
                                     : 0);
    put_i16(&response[7], current_valid ? static_cast<int16_t>(current_ma) : 0);
    const int64_t power_mw = power_valid
                                     ? (int64_t(response[3] << 8 | response[4]) *
                                        current.current_ua) /
                                               1000000
                                     : 0;
    put_i32(&response[9], power_valid ? static_cast<int32_t>(power_mw) : 0);
    response[13] = dock.valid ? static_cast<uint8_t>(dock.state)
                              : static_cast<uint8_t>(DockPowerState::UNKNOWN);
    put_u16(&response[14], age_deciseconds(fuel.valid, fuel.age_ms));
    put_u16(&response[16], age_deciseconds(current.valid, current.age_ms));
    put_u16(&response[18], age_deciseconds(dock.valid, dock.age_ms));
}
