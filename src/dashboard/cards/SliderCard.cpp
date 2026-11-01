#include "SliderCard.h"

SliderCard::SliderCard(const String& id, const String& title, int min, int max, int step, const String& unit)
    : DashboardCard(id, CardType::SLIDER, title), min(min), max(max), step(step), unit(unit), value(min) {}

void SliderCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    config["min"] = min;
    config["max"] = max;
    config["step"] = step;
    config["unit"] = unit;
    writeSize(config);
}

void SliderCard::handleAction(const String& action, JsonObject& data) {
    if (action == "change" && !data["value"].isNull()) {
        value = data["value"].as<int>();
        if (onChange) {
            onChange(value);
        }
    }
}

void SliderCard::setValue(int val) {
    value = constrain(val, min, max);
}
