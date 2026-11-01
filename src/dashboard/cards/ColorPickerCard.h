#pragma once

#include "../DashboardCard.h"

class ColorPickerCard : public DashboardCard {
public:
    String value;
    std::vector<String> presets;
    ColorCallback onChange;

    ColorPickerCard(const String& id, const String& title, const String& defaultColor = "#00D4AA");

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setPresets(std::vector<String>& p);

    void setValue(const String& val);
};
