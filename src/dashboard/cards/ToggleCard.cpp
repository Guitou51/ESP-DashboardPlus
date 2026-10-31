#include "ToggleCard.h"

ToggleCard::ToggleCard(const String& id, const String& title, const String& label, bool defaultValue)
    : DashboardCard(id, CardType::TOGGLE, title), label(label), value(defaultValue) {}

void ToggleCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["label"] = label;
    config["value"] = value;
    writeSize(config);
}

void ToggleCard::handleAction(const String& action, JsonObject& data) {
    if (action == "change" && !data["value"].isNull()) {
        value = data["value"].as<bool>();
        if (onChange) {
            onChange(value);
        }
    }
}

void ToggleCard::setValue(bool val) {
    value = val;
}
