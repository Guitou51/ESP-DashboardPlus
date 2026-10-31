#pragma once

#include "MessageRouter.h"
#include <memory>

class AsyncWebServer;
class AsyncWebSocket;

namespace espdashboardplus {
namespace detail {

// ESPAsyncWebServer adapter: HTTP hosting, WebSocket frames and connection upkeep.
class AsyncDashboardTransport {
public:
    using Connected = std::function<void(const MessageReply&)>;
    using Received = std::function<void(const uint8_t*, size_t, const MessageReply&)>;
    AsyncDashboardTransport();
    ~AsyncDashboardTransport();
    void begin(AsyncWebServer* server, const uint8_t* html, size_t size, const String& path, Connected connected,
               Received received);
    bool ready() const;
    void broadcast(const String& message);
    void loop();

private:
    AsyncWebSocket* _socket;
    const uint8_t* _html;
    size_t _htmlSize;
    unsigned long _lastHeartbeat;
    std::unique_ptr<uint8_t[]> _buffer;
    size_t _bufferSize;
    size_t _bufferFilled;
};

} // namespace detail
} // namespace espdashboardplus
