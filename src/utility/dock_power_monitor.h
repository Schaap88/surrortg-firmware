#ifndef DOCK_POWER_MONITOR_H
#define DOCK_POWER_MONITOR_H

#include <stdint.h>

enum class DockPowerState : uint8_t { UNKNOWN = 0, ABSENT = 1, PRESENT = 2 };

struct DockPowerTelemetry {
    DockPowerState state;
    uint32_t age_ms;
    bool valid;
};

void init_dock_power_monitor(uint8_t pin, unsigned long now_ms);
void poll_dock_power_monitor(unsigned long now_ms);
DockPowerTelemetry read_dock_power_telemetry(unsigned long now_ms);

#endif  // DOCK_POWER_MONITOR_H
