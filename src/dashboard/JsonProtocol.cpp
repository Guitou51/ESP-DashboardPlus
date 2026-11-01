#include "JsonProtocol.h"
#include "CardRegistry.h"

namespace espdashboardplus {
namespace detail {
namespace {
String encode(const JsonDocument& document) {
    String message;
    serializeJson(document, message);
    return message;
}
} // namespace

String snapshot(const DashboardState& state, const CardRegistry& registry) {
    JsonDocument doc;
    doc["type"] = "init";
    doc["title"] = state.title;
    if (state.subtitle.length() > 0)
        doc["subtitle"] = state.subtitle;
    doc["enableOTA"] = state.enableOTA;
    doc["enableConsole"] = state.enableConsole;
    if (state.version.length() > 0)
        doc["version"] = state.version;
    if (state.lastUpdate.length() > 0)
        doc["lastUpdate"] = state.lastUpdate;
    auto cards = doc["cards"].to<JsonArray>();
    for (const auto& pair : registry.cards()) {
        auto card = cards.add<JsonObject>();
        pair.second->toJson(card);
    }
    if (!registry.groups().empty()) {
        auto groups = doc["groups"].to<JsonArray>();
        for (const auto& group : registry.groups()) {
            auto object = groups.add<JsonObject>();
            object["id"] = group.id;
            object["title"] = group.title;
            auto ids = object["cardIds"].to<JsonArray>();
            for (const auto& id : group.cardIds)
                ids.add(id);
        }
    }
    return encode(doc);
}

String updateMessage(const String& id, JsonObject data) {
    JsonDocument doc;
    doc["type"] = "update";
    doc["cardId"] = id;
    doc["data"] = data;
    return encode(doc);
}

String removeMessage(const String& id) {
    JsonDocument doc;
    doc["type"] = "remove";
    doc["cardId"] = id;
    return encode(doc);
}

String heartbeatMessage(unsigned long timestamp) {
    JsonDocument doc;
    doc["type"] = "heartbeat";
    doc["timestamp"] = timestamp;
    return encode(doc);
}

String logMessage(LogLevel level, const String& message, unsigned long timestamp) {
    JsonDocument doc;
    doc["type"] = "log";
    doc["timestamp"] = formatTimestamp(timestamp);
    doc["level"] = logLevelName(level);
    doc["message"] = message;
    return encode(doc);
}

} // namespace detail
} // namespace espdashboardplus
