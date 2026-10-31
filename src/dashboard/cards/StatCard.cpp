#include "StatCard.h"

StatCard::StatCard(const String& id, const String& title, const String& value, const String& unit)
    : DashboardCard(id, CardType::STAT, title), value(value), unit(unit) {}

void StatCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    config["unit"] = unit;
    config["color"] = variantToString(variant);
    writeSize(config);
    if (trend.length() > 0) {
        config["trend"] = trend;
        config["trendValue"] = trendValue;
    }
}

void StatCard::setValue(const String& val) {
    value = val;
}

void StatCard::setTrend(const String& t, const String& val) {
    trend = t;
    trendValue = val;
}
