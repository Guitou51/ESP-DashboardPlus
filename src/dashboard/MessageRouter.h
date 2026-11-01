#pragma once

#include "JsonProtocol.h"

namespace espdashboardplus {
namespace detail {
class CardRegistry;
class OtaSession;
using MessageReply = std::function<void(const String&)>;

// Routes protocol messages without depending on ESPAsyncWebServer.
class MessageRouter {
public:
    MessageRouter(CardRegistry& cards, OtaSession& ota, DashboardState& state);
    void initialize(const MessageReply& reply);
    void handle(const uint8_t* data, size_t size, const MessageReply& reply, uint32_t clientId = 0);

private:
    CardRegistry& _cards;
    OtaSession& _ota;
    DashboardState& _state;
};

} // namespace detail
} // namespace espdashboardplus
