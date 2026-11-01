#pragma once

#include "../DashboardCard.h"

class LinkCard : public DashboardCard {
public:
    String label;
    String url;
    String target; // "_blank", "_self"
    String icon;

    LinkCard(const String& id, const String& title, const String& label, const String& url);

    void toJson(JsonObject& card) override;

    void setTarget(const String& t);

    void setIcon(const String& i);

    void setUrl(const String& u);
};
