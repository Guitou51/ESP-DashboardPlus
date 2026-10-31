#pragma once

#include <Arduino.h>

namespace espdashboardplus {
namespace detail {

// The OTA session depends on this small port instead of the ESP32 Update singleton.
class FirmwareUpdate {
public:
    virtual ~FirmwareUpdate() = default;
    virtual bool supported() const = 0;
    virtual bool begin(size_t size) = 0;
    virtual size_t write(uint8_t* data, size_t size) = 0;
    virtual bool end() = 0;
    virtual void abort() = 0;
    virtual void printError() = 0;
    virtual void restart(unsigned long delayMs) = 0;
};

class ArduinoFirmwareUpdate : public FirmwareUpdate {
public:
    bool supported() const override;
    bool begin(size_t size) override;
    size_t write(uint8_t* data, size_t size) override;
    bool end() override;
    void abort() override;
    void printError() override;
    void restart(unsigned long delayMs) override;
};

} // namespace detail
} // namespace espdashboardplus
