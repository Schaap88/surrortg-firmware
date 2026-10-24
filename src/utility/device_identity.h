#ifndef SRTG_DEVICE_IDENTITY_H
#define SRTG_DEVICE_IDENTITY_H

#include <stddef.h>
#include <stdint.h>

constexpr uint8_t SRTG_DEVICE_IDENTITY_VERSION = 1;
constexpr size_t SRTG_DEVICE_IDENTITY_RESPONSE_SIZE = 8;

void serialize_device_identity(
        uint8_t command_id, const uint8_t station_mac[6],
        uint8_t response[SRTG_DEVICE_IDENTITY_RESPONSE_SIZE]);

#endif  // SRTG_DEVICE_IDENTITY_H
