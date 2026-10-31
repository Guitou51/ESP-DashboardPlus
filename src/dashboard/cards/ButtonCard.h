#pragma once

#include "../DashboardCard.h"

class ButtonCardImpl : public DashboardCard {
public:
    String label;
    String icon;
    ButtonCallback callback;

    ButtonCardImpl(const String& id, const String& title, const String& label, ButtonCallback cb);

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setIcon(const String& i);
};
