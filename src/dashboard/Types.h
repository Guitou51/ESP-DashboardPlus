#pragma once

#include <Arduino.h>
#include <functional>
#include <vector>

// Callback types
typedef std::function<void()> ButtonCallback;
typedef std::function<void(const String&)> InputCallback;
typedef std::function<void(bool)> ToggleCallback;
typedef std::function<void(int)> SliderCallback;
typedef std::function<void(const String&)> ColorCallback;
typedef std::function<void(const String&)> DropdownCallback;
typedef std::function<void(const String&, int, const String&)> TimezoneCallback;
typedef std::function<void(const String&)> DateCallback;
typedef std::function<void(const String&)> TimeCallback;
typedef std::function<void(float, float)> LocationCallback;

// Card types
enum class CardType {
    STAT,
    CHART,
    BUTTON,
    ACTION,
    DATA_INPUT,
    COLOR,
    DROPDOWN,
    OTA,
    GAUGE,
    TOGGLE,
    SLIDER,
    LINK,
    TIMEZONE,
    DATE,
    TIME,
    LOCATION,
    STATUS,
    CONSOLE
};

// Log levels for console
enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

// Log entry structure
struct LogEntry {
    String timestamp;
    LogLevel level;
    String message;
};

// Card variant/color
enum class CardVariant { PRIMARY, SUCCESS, WARNING, DANGER, INFO, SECONDARY };

// Chart types
enum class ChartType { LINE, AREA, BAR, SCATTER, STEP };

// Status icons
enum class StatusIcon { CHECK, ERROR, WARNING, INFO, WIFI, POWER, SYNC, CLOUD, LOCK, UNLOCK };

// Dropdown option
struct DropdownOption {
    String value;
    String label;
};

struct ChartSeries {
    String name;
    String color; // "primary", "success", "warning", "danger", "info"
    std::vector<float> data;

    ChartSeries(const String& name = "", const String& color = "primary");
};

struct CardGroup {
    String id;
    String title;
    std::vector<String> cardIds;

    CardGroup(const String& id, const String& title);
};

namespace espdashboardplus {
namespace detail {
String variantName(CardVariant value);
String cardTypeName(CardType value);
String logLevelName(LogLevel value);
String chartTypeName(ChartType value);
String iconName(StatusIcon value);
String formatTimestamp(unsigned long milliseconds);
} // namespace detail
} // namespace espdashboardplus
