#ifndef SRTG_POWER_STATUS_H
#define SRTG_POWER_STATUS_H

#include <stddef.h>
#include <stdint.h>

#include "dock_power_monitor.h"
#include "ina226_power_monitor.h"
#include "max17043_battery_manager.h"

constexpr size_t SRTG_POWER_STATUS_RESPONSE_SIZE = 20;
constexpr uint8_t SRTG_POWER_STATUS_VERSION = 1;
constexpr uint16_t SRTG_POWER_AGE_UNAVAILABLE = 0xFFFF;

enum PowerStatusValidity : uint8_t {
    POWER_VOLTAGE_VALID = 1 << 0,
    POWER_SOC_VALID = 1 << 1,
    POWER_CURRENT_VALID = 1 << 2,
    POWER_POWER_VALID = 1 << 3,
    POWER_DOCK_VALID = 1 << 4,
};

void serialize_power_status(uint8_t command_id,
                            const Max17048Telemetry& fuel,
                            const Ina226Telemetry& current,
                            const DockPowerTelemetry& dock,
                            uint8_t response[SRTG_POWER_STATUS_RESPONSE_SIZE]);

#endif  // SRTG_POWER_STATUS_H
