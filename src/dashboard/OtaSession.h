#pragma once

#include "FirmwareUpdate.h"
#include <ArduinoJson.h>
#include <functional>
#include <mutex>

namespace espdashboardplus {
namespace detail {

using OtaReply = std::function<void(const String&)>;

class OtaSession {
public:
    explicit OtaSession(FirmwareUpdate& firmware);
    bool handle(const String& action, JsonObject data, const OtaReply& reply = {}, uint32_t clientId = 0);
    void disconnect(uint32_t clientId);
    void poll(unsigned long now);

private:
    FirmwareUpdate& _firmware;
    std::mutex _mutex;
    bool _inProgress = false;
    bool _restartPending = false;
    size_t _size = 0;
    size_t _received = 0;
    uint32_t _owner = 0;
    String _uploadId;
    size_t _nextSequence = 0;
    unsigned long _lastActivity = 0;
    unsigned long _completedAt = 0;
    String start(size_t size);
    String chunk(const String& encoded);
    String finish();
    void abort();
    String statusMessage(const String& action, const String& uploadId, size_t sequence, const String& stage,
                         const String& error) const;
};

} // namespace detail
} // namespace espdashboardplus
