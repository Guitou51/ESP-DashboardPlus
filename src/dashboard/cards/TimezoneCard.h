#pragma once

#include "../DashboardCard.h"

class TimezoneCard : public DashboardCard {
public:
    String value;
    String label;
    TimezoneCallback callback;

    TimezoneCard(const String& id, const String& title, const String& label = "Get Browser Timezone");

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setCallback(TimezoneCallback cb);
};
