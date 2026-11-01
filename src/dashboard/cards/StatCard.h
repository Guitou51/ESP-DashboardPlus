#pragma once

#include "../DashboardCard.h"

class StatCard : public DashboardCard {
public:
    String value;
    String unit;
    String trend; // "up", "down", or ""
    String trendValue;

    StatCard(const String& id, const String& title, const String& value = "", const String& unit = "");

    void toJson(JsonObject& card) override;

    void setValue(const String& val);

    void setTrend(const String& t, const String& val);
};
