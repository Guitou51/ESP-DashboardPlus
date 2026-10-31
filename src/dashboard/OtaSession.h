#pragma once

#include "FirmwareUpdate.h"
#include <ArduinoJson.h>

namespace espdashboardplus {
namespace detail {

// One implementation for both legacy card actions and dedicated OTA messages.
class OtaSession {
public:
    explicit OtaSession(FirmwareUpdate& firmware);
    bool handle(const String& action, JsonObject data);

private:
    FirmwareUpdate& _firmware;
    bool _inProgress = false;
    size_t _size = 0;
    size_t _received = 0;
    void start(size_t size);
    void chunk(const String& encoded);
    void finish();
    void abort();
};

} // namespace detail
} // namespace espdashboardplus
