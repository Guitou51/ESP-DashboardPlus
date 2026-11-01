#pragma once

#include "../DashboardCard.h"

class DateCard : public DashboardCard {
public:
    String value;
    bool includeTime;
    String minDate;
    String maxDate;
    DateCallback callback;

    DateCard(const String& id, const String& title, bool includeTime = false);

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setCallback(DateCallback cb);

    void setRange(const String& min, const String& max);

    void setValue(const String& val);
};
