# SurroRTG Arduino library

This library provides tools to control WiFi microcontrollers using the SRTG TCP protocol.
Supported microcontrollers include ESP32 and ESP8266 based boards, as well
as Arduino MKR1000 and Arduino MKR1010.

On ESP32, `setup_sdk()` initializes ArduinoOTA and `process_sdk()` services it;
the consuming firmware must call `process_sdk()` regularly and provide an
OTA-capable partition layout and matching uploader configuration. ArduinoOTA
uses default port **3232**, separate from the SRTG command port **31338**.
The ESP32 partition table must contain `otadata` and two application slots
(`ota_0` and `ota_1`), each large enough for the firmware. Existing devices
with a single-slot layout need a serial flash of the new partition table and
firmware before OTA can be used. See the [examples](examples) for library usage.

The application's `reset_cb` is the control-safety callback. It is invoked for
an explicit STOP, the five-second command timeout, client disconnection, WiFi
loss/restart and OTA start. Buffered bytes from a disconnected client are
discarded and cannot start a new control session.

For ESP32/ESP8266 battery monitoring, the application owns and initializes the
I2C bus passed to `setup_sdk()`. The historical MAX17043 API drives the
register-compatible MAX17048 fitted to Power Board Rev A and polls every ten seconds;
battery requests use a cached reading no more than thirty seconds old and do
not perform I2C work in the TCP request path. If no fresh, checked reading is
available, no battery packet is emitted because the existing packet format has
no validity field.

When built with `SRTG_POWER_TELEMETRY`, command `101` (`POWER_STATUS`) adds a
versioned 20-byte response. All multibyte fields are big-endian; signed fields
use two's-complement encoding:

| Offset | Width | Field |
| ---: | ---: | --- |
| 0 | 1 | command ID (`101`) |
| 1 | 1 | format version (`1`) |
| 2 | 1 | validity bits: voltage, SoC, current, power, dock in bits 0..4 |
| 3 | 2 | battery voltage, unsigned mV |
| 5 | 2 | state of charge, unsigned 0.01% |
| 7 | 2 | battery current, signed mA; positive discharge, negative charge |
| 9 | 4 | battery power, signed mW; same sign convention as current |
| 13 | 1 | dock: `0` unknown, `1` absent, `2` present |
| 14 | 2 | MAX17048 sample age in 100 ms units |
| 16 | 2 | INA226 sample age in 100 ms units |
| 18 | 2 | dock sample age in 100 ms units |

An age of `0xffff` explicitly means unavailable; `0xfffe` is the greatest
representable age. Numeric fields whose validity bit is clear must be ignored.
The INA226 is polled once per second and expires after three seconds; its Rev A
configuration is `0x4527`, current LSB is 100 uA and calibration is `0x1400`.
GPIO15 dock detection uses plain `INPUT` (no internal pull-up) and 50 ms
debounce. Dock presence only reports external power availability and is not
evidence that the battery is accepting charge.

The legacy `BATTERY_STATUS` command remains ID `100` with its exact nine-byte
native-float response. Existing clients do not need to change.

## Adding wifi and OTA credentials for examples

Before the examples can be compiled and uploaded, the examples need wifi credentials
and password for Over-The-Air (OTA) code updates. These can be set by making a
new file called `wifi_secrets.h` and placing it to the same directory with the
example code. If your Arduino IDE opens the example read-only, save a copy to
a writable location first.
Then, add the following content to the newly created file and change "ssid" to your
wifi name and "password" to your wifi password. Change the value of ota_password to
a password that you would like to use to secure your OTA uploads.

```c
#ifndef _SURRORTG_WIFI_SECRETS_H
#define _SURRORTG_WIFI_SECRETS_H

constexpr const char* ssid = "";            // Network name here
constexpr const char* password = "";        // Network password here
constexpr const char* ota_password = "";    // OTA-update password here

#endif  // _SURRORTG_WIFI_SECRETS_H
```

If the filename is incorrect or the file is missing, building the code
outputs an error `wifi_secrets.h: No such file or directory`.

## Development

### Code style

The project uses `clang-format` tool to make sure the code follows a specific
style. The formatting is checked on every push in the Github Actions. If the
Actions fail, please fix the formatting of the files that you have changed by
running `clang-format --style=file -i <filenames>`, where `<filenames>` are the
names of the files you want to format, separated by spaces. Alternatively you
can use a plugin on your code editor that supports `clang-format`. Remember to
commit the changes after formatting.

You can also run the following command in the root of the repository to format
all the .cpp, .h and .ino files in the repository:

```shell
find . -type f \( -name \*.cpp -o -name \*.h -o -name \*.ino \) -exec clang-format --style=file -i "{}" \;
```
