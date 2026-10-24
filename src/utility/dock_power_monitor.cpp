#include <Arduino.h>

#include "dock_power_monitor.h"

namespace {
constexpr unsigned long DEBOUNCE_MS = 50;
uint8_t dock_pin = 0;
int candidate_level = HIGH;
unsigned long candidate_since_ms = 0;
unsigned long sampled_at_ms = 0;
DockPowerState stable_state = DockPowerState::UNKNOWN;
bool initialized = false;
}  // namespace

void init_dock_power_monitor(uint8_t pin, unsigned long now_ms) {
    dock_pin = pin;
    pinMode(dock_pin, INPUT);  // Rev A supplies the pull-up; GPIO15 gets none.
    candidate_level = digitalRead(dock_pin);
    candidate_since_ms = now_ms;
    sampled_at_ms = now_ms;
    stable_state = DockPowerState::UNKNOWN;
    initialized = true;
}

void poll_dock_power_monitor(unsigned long now_ms) {
    if (!initialized) return;
    const int level = digitalRead(dock_pin);
    sampled_at_ms = now_ms;
    if (level != candidate_level) {
        candidate_level = level;
        candidate_since_ms = now_ms;
        return;
    }
    if (now_ms - candidate_since_ms >= DEBOUNCE_MS) {
        stable_state = level == LOW ? DockPowerState::PRESENT
                                    : DockPowerState::ABSENT;
    }
}

DockPowerTelemetry read_dock_power_telemetry(unsigned long now_ms) {
    if (!initialized || stable_state == DockPowerState::UNKNOWN)
        return {DockPowerState::UNKNOWN, 0, false};
    return {stable_state, static_cast<uint32_t>(now_ms - sampled_at_ms), true};
}
