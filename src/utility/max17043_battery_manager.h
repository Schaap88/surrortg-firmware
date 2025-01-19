#ifndef MAX17043_BATTERY_MANAGER_H
#define MAX17043_BATTERY_MANAGER_H

#include <Wire.h>

/* Structure to hold battery status information */
struct BatteryStatus {
    float voltage; // Voltage in volts
    float soc;     // State of charge in percentage
};

/* Initializes the MAX17043 battery manager */
bool init_max17043();

/* Reads the current battery status */
bool read_battery_status(BatteryStatus* status);

#endif // MAX17043_BATTERY_MANAGER_H