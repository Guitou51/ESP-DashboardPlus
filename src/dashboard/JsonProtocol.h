#pragma once

#include "Types.h"
#include <ArduinoJson.h>

namespace espdashboardplus {
namespace detail {
class CardRegistry;

struct DashboardState {
    String title;
    String subtitle;
    String version;
    String lastUpdate;
    bool enableOTA = true;
    bool enableConsole = true;
    std::function<void(const String&)> onCommand;
};

String snapshot(const DashboardState& state, const CardRegistry& registry);
String updateMessage(const String& id, JsonObject data);
String removeMessage(const String& id);
String heartbeatMessage(unsigned long timestamp);
String logMessage(LogLevel level, const String& message, unsigned long timestamp);

} // namespace detail
} // namespace espdashboardplus
