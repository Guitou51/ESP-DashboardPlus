#include "ConsoleCard.h"

ConsoleCard::ConsoleCard(const String& id, const String& title, int maxEntries)
    : DashboardCard(id, CardType::CONSOLE, title), maxEntries(maxEntries), autoScroll(true) {}

void ConsoleCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["autoScroll"] = autoScroll;
    JsonArray logsArr = config["logs"].to<JsonArray>();
    for (const LogEntry& entry : logs) {
        JsonObject logObj = logsArr.add<JsonObject>();
        logObj["timestamp"] = entry.timestamp;
        logObj["level"] = logLevelToString(entry.level);
        logObj["message"] = entry.message;
    }
}

void ConsoleCard::handleAction(const String& action, JsonObject& data) {
    if (action == "clear") {
        logs.clear();
        if (onClear)
            onClear();
    } else if (action == "command" && !data["command"].isNull()) {
        String cmd = data["command"].as<String>();
        if (onCommand)
            onCommand(cmd);
    }
}

void ConsoleCard::addLog(LogLevel level, const String& message) {
    LogEntry entry;
    entry.timestamp = getTimestamp();
    entry.level = level;
    entry.message = message;

    logs.push_back(entry);

    // Trim old entries if exceeding max
    while (logs.size() > maxEntries) {
        logs.erase(logs.begin());
    }
}

void ConsoleCard::clear() {
    logs.clear();
}

String ConsoleCard::getLogLevelString(LogLevel level) {
    return logLevelToString(level);
}

String ConsoleCard::getTimestamp() {
    return espdashboardplus::detail::formatTimestamp(millis());
}
