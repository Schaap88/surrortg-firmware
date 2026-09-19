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
