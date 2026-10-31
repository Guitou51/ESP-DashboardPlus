#pragma once

#include "../DashboardCard.h"

class StatusCard : public DashboardCard {
public:
    StatusIcon icon;
    String label;
    String message;

    StatusCard(const String& id, const String& title, StatusIcon icon = StatusIcon::INFO);

    void toJson(JsonObject& card) override;

    void setStatus(StatusIcon i, CardVariant v, const String& lbl, const String& msg);

    void setLabel(const String& lbl);

    void setMessage(const String& msg);

    void setIcon(StatusIcon i);
};
