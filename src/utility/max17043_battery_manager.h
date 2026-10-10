#ifndef MAX17043_BATTERY_MANAGER_H
#define MAX17043_BATTERY_MANAGER_H

#include <Wire.h>
#include "max17043_conversion.h"

/* Structure to hold battery status information */
struct BatteryStatus {
    float voltage; // Voltage in volts
    float soc;     // State of charge in percentage
};

/* Initializes the MAX17043 battery manager on an application-owned I2C bus. */
bool init_max17043(TwoWire& wire);

/* Refreshes the cached reading when its polling interval has elapsed. */
void poll_max17043(unsigned long now_ms);

/* Returns a recent cached reading without performing I2C transactions. */
bool read_battery_status(BatteryStatus* status);

#endif // MAX17043_BATTERY_MANAGER_H
