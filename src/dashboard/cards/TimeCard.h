#pragma once

#include "../DashboardCard.h"

class TimeCard : public DashboardCard {
public:
    String value;
    bool includeSeconds;
    TimeCallback callback;

    TimeCard(const String& id, const String& title, bool includeSeconds = false);

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setCallback(TimeCallback cb);

    void setValue(const String& val);
};
