#pragma once

#include "../DashboardCard.h"

class SliderCard : public DashboardCard {
public:
    int value;
    int min, max, step;
    String unit;
    SliderCallback onChange;

    SliderCard(const String& id, const String& title, int min = 0, int max = 100, int step = 1,
               const String& unit = "");

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setValue(int val);
};
