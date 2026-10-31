#pragma once

#include "../DashboardCard.h"

class InputCard : public DashboardCard {
public:
    String value;
    String placeholder;
    String inputType; // "text", "number", "password"
    String unit;
    float min, max, step;
    InputCallback onChange;
    InputCallback onSubmit;

    InputCard(const String& id, const String& title, const String& placeholder = "");

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setNumberInput(float minVal, float maxVal, float stepVal, const String& unitStr = "");

    void setValue(const String& val);
};
