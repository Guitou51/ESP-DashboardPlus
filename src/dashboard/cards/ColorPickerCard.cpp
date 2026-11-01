#include "ColorPickerCard.h"

ColorPickerCard::ColorPickerCard(const String& id, const String& title, const String& defaultColor)
    : DashboardCard(id, CardType::COLOR, title), value(defaultColor) {
    // Default presets
    presets = {"#00D4AA", "#22C55E", "#F59E0B", "#F97316", "#EF4444",
               "#EC4899", "#8B5CF6", "#3B82F6", "#06B6D4", "#FFFFFF"};
}

void ColorPickerCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    JsonArray presetsArr = config["presets"].to<JsonArray>();
    for (const String& preset : presets) {
        presetsArr.add(preset);
    }
    writeSize(config);
}

void ColorPickerCard::handleAction(const String& action, JsonObject& data) {
    if (action == "change" && !data["color"].isNull()) {
        value = data["color"].as<String>();
        if (onChange) {
            onChange(value);
        }
    }
}

void ColorPickerCard::setPresets(std::vector<String>& p) {
    presets = p;
}

void ColorPickerCard::setValue(const String& val) {
    value = val;
}
