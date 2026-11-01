#pragma once

#include "../DashboardCard.h"

class ToggleCard : public DashboardCard {
public:
    bool value;
    String label;
    ToggleCallback onChange;

    ToggleCard(const String& id, const String& title, const String& label = "", bool defaultValue = false);

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setValue(bool val);
};
