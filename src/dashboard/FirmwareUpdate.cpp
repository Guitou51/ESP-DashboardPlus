#include "FirmwareUpdate.h"
#if defined(ARDUINO_ARCH_ESP32)
#include <Update.h>
#endif

namespace espdashboardplus {
namespace detail {

bool ArduinoFirmwareUpdate::supported() const {
#if defined(ARDUINO_ARCH_ESP32)
    return true;
#else
    return false;
#endif
}

bool ArduinoFirmwareUpdate::begin(size_t size) {
#if defined(ARDUINO_ARCH_ESP32)
    return Update.begin(size);
#else
    return false;
#endif
}

size_t ArduinoFirmwareUpdate::write(uint8_t* data, size_t size) {
#if defined(ARDUINO_ARCH_ESP32)
    return Update.write(data, size);
#else
    return 0;
#endif
}

bool ArduinoFirmwareUpdate::end() {
#if defined(ARDUINO_ARCH_ESP32)
    return Update.end(true);
#else
    return false;
#endif
}

void ArduinoFirmwareUpdate::abort() {
#if defined(ARDUINO_ARCH_ESP32)
    Update.abort();
#endif
}

void ArduinoFirmwareUpdate::printError() {
#if defined(ARDUINO_ARCH_ESP32)
    Update.printError(Serial);
#endif
}

void ArduinoFirmwareUpdate::restart(unsigned long delayMs) {
    delay(delayMs);
    ESP.restart();
}

} // namespace detail
} // namespace espdashboardplus
