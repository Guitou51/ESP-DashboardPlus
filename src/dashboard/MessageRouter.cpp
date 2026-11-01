#include "MessageRouter.h"
#include "CardRegistry.h"
#include "OtaSession.h"

namespace espdashboardplus {
namespace detail {

MessageRouter::MessageRouter(CardRegistry& cards, OtaSession& ota, DashboardState& state)
    : _cards(cards), _ota(ota), _state(state) {}

void MessageRouter::initialize(const MessageReply& reply) {
    reply(snapshot(_state, _cards));
}

void MessageRouter::handle(const uint8_t* bytes, size_t size, const MessageReply& reply, uint32_t clientId) {
    if (!bytes || size == 0)
        return;
    JsonDocument doc;
    const auto error = deserializeJson(doc, reinterpret_cast<const char*>(bytes), size);
    if (error) {
        Serial.print("[Dashboard] JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }
    const String type = doc["type"].as<String>();
    if (type == "init") {
        initialize(reply);
    } else if (type == "action") {
        auto* card = _cards.get(doc["cardId"].as<String>());
        if (card) {
            const String action = doc["action"].as<String>();
            auto data = doc["data"].as<JsonObject>();
            card->handleAction(action, data);
            _ota.handle(action, data, reply, clientId);
        }
    } else if (type == "command") {
        const String command = doc["command"].as<String>();
        if (_state.onCommand)
            _state.onCommand(command);
    } else {
        _ota.handle(type, doc.as<JsonObject>(), reply, clientId);
    }
}

} // namespace detail
} // namespace espdashboardplus
