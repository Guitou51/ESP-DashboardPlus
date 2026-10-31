#pragma once

#include "../DashboardCard.h"

class ConsoleCard : public DashboardCard {
public:
    std::vector<LogEntry> logs;
    int maxEntries;
    bool autoScroll;
    std::function<void()> onClear;
    std::function<void(const String&)> onCommand;

    ConsoleCard(const String& id, const String& title = "Console Log", int maxEntries = 100);

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void addLog(LogLevel level, const String& message);

    void clear();

    // Public accessor for log level string conversion
    String getLogLevelString(LogLevel level);

private:
    String getTimestamp();
};
