#ifndef MAX17043_BATTERY_MANAGER_H
#define MAX17043_BATTERY_MANAGER_H

#include <Wire.h>
#include <stddef.h>
#include "max17043_conversion.h"

/* Structure to hold battery status information */
struct BatteryStatus {
    float voltage; // Voltage in volts
    float soc;     // State of charge in percentage
};

struct Max17048Telemetry {
    BatteryStatus status;
    uint32_t age_ms;
    bool valid;
};

/* Initializes the MAX17043 battery manager on an application-owned I2C bus. */
bool init_max17043(TwoWire& wire);

/* Refreshes the cached reading when its polling interval has elapsed. */
void poll_max17043(unsigned long now_ms);

/* Returns a recent cached reading without performing I2C transactions. */
bool read_battery_status(BatteryStatus* status);

/* Detailed MAX17048 status for the additive power telemetry response. */
Max17048Telemetry read_max17048_telemetry(unsigned long now_ms);

constexpr size_t LEGACY_BATTERY_STATUS_RESPONSE_SIZE = 9;
void serialize_legacy_battery_status(
        uint8_t command_id, const BatteryStatus& status,
        uint8_t response[LEGACY_BATTERY_STATUS_RESPONSE_SIZE]);

#endif // MAX17043_BATTERY_MANAGER_H
