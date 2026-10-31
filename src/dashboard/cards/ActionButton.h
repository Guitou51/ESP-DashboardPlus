#pragma once

#include "../DashboardCard.h"

class ActionButton : public DashboardCard {
public:
    String label;
    String confirmTitle;
    String confirmMessage;
    String icon;
    ButtonCallback callback;

    ActionButton(const String& id, const String& title, const String& label, const String& confirmTitle,
                 const String& confirmMsg, ButtonCallback cb);

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setIcon(const String& i);
};
