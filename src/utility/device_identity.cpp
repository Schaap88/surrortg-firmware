#include "device_identity.h"

void serialize_device_identity(
        uint8_t command_id, const uint8_t station_mac[6],
        uint8_t response[SRTG_DEVICE_IDENTITY_RESPONSE_SIZE]) {
    response[0] = command_id;
    response[1] = SRTG_DEVICE_IDENTITY_VERSION;
    for (size_t index = 0; index < 6; ++index) {
        response[index + 2] = station_mac[index];
    }
}
