#include <Wire.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
#include "max17043_battery_manager.h"

static SFE_MAX1704X max17043(MAX1704X_MAX17043);

bool init_max17043() {
    Wire.begin(); // Initialize I2C
    if (max17043.begin(Wire)) {
        Serial.println("MAX17043 initialized successfully");
        return true;
    } else {
        Serial.println("Failed to initialize MAX17043");
        return false;
    }
}

bool read_battery_status(BatteryStatus* status) {
    if (!status) {
        return false;
    }
    status->voltage = max17043.getVoltage();
    status->soc = max17043.getSOC();
    return true;
}
