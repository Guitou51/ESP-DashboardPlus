#include "GaugeCard.h"

GaugeCard::GaugeCard(const String& id, const String& title, float min, float max, const String& unit)
    : DashboardCard(id, CardType::GAUGE, title), min(min), max(max), unit(unit), value(min), warningThreshold(70),
      dangerThreshold(90) {}

void GaugeCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    config["min"] = min;
    config["max"] = max;
    config["unit"] = unit;
    JsonObject thresholds = config["thresholds"].to<JsonObject>();
    thresholds["warning"] = warningThreshold;
    thresholds["danger"] = dangerThreshold;
    writeSize(config);
}

void GaugeCard::setValue(float val) {
    value = constrain(val, min, max);
}

void GaugeCard::setThresholds(float warning, float danger) {
    warningThreshold = warning;
    dangerThreshold = danger;
}
