#include "Types.h"
#include <cstdio>

ChartSeries::ChartSeries(const String& name, const String& color) : name(name), color(color) {}

CardGroup::CardGroup(const String& id, const String& title) : id(id), title(title) {}

namespace espdashboardplus {
namespace detail {

String variantName(CardVariant v) {
    switch (v) {
        case CardVariant::SUCCESS:
            return "success";
        case CardVariant::WARNING:
            return "warning";
        case CardVariant::DANGER:
            return "danger";
        case CardVariant::INFO:
            return "info";
        case CardVariant::SECONDARY:
            return "secondary";
        default:
            return "primary";
    }
}

String cardTypeName(CardType t) {
    switch (t) {
        case CardType::STAT:
            return "stat";
        case CardType::CHART:
            return "chart";
        case CardType::BUTTON:
            return "button";
        case CardType::ACTION:
            return "action";
        case CardType::DATA_INPUT:
            return "input";
        case CardType::COLOR:
            return "color";
        case CardType::DROPDOWN:
            return "dropdown";
        case CardType::OTA:
            return "ota";
        case CardType::GAUGE:
            return "gauge";
        case CardType::TOGGLE:
            return "toggle";
        case CardType::SLIDER:
            return "slider";
        case CardType::LINK:
            return "link";
        case CardType::TIMEZONE:
            return "timezone";
        case CardType::DATE:
            return "date";
        case CardType::TIME:
            return "time";
        case CardType::LOCATION:
            return "location";
        case CardType::STATUS:
            return "status";
        case CardType::CONSOLE:
            return "console";
        default:
            return "stat";
    }
}

String logLevelName(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "debug";
        case LogLevel::INFO:
            return "info";
        case LogLevel::WARNING:
            return "warning";
        case LogLevel::ERROR:
            return "error";
        default:
            return "info";
    }
}

String chartTypeName(ChartType t) {
    switch (t) {
        case ChartType::AREA:
            return "area";
        case ChartType::BAR:
            return "bar";
        case ChartType::SCATTER:
            return "scatter";
        case ChartType::STEP:
            return "step";
        default:
            return "line";
    }
}

String iconName(StatusIcon i) {
    switch (i) {
        case StatusIcon::CHECK:
            return "check";
        case StatusIcon::ERROR:
            return "error";
        case StatusIcon::WARNING:
            return "warning";
        case StatusIcon::INFO:
            return "info";
        case StatusIcon::WIFI:
            return "wifi";
        case StatusIcon::POWER:
            return "power";
        case StatusIcon::SYNC:
            return "sync";
        case StatusIcon::CLOUD:
            return "cloud";
        case StatusIcon::LOCK:
            return "lock";
        case StatusIcon::UNLOCK:
            return "unlock";
        default:
            return "info";
    }
}

String formatTimestamp(unsigned long ms) {
    const unsigned long seconds = ms / 1000;
    char text[16];
    snprintf(text, sizeof(text), "%02lu:%02lu:%02lu.%03lu", (seconds / 3600) % 24, (seconds / 60) % 60, seconds % 60,
             ms % 1000);
    return String(text);
}

} // namespace detail
} // namespace espdashboardplus
