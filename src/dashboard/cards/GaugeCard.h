#pragma once

#include "../DashboardCard.h"

class GaugeCard : public DashboardCard {
public:
    float value;
    float min, max;
    String unit;
    float warningThreshold;
    float dangerThreshold;

    GaugeCard(const String& id, const String& title, float min = 0, float max = 100, const String& unit = "%");

    void toJson(JsonObject& card) override;

    void setValue(float val);

    void setThresholds(float warning, float danger);
};
