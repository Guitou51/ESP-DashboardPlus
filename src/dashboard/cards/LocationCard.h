#pragma once

#include "../DashboardCard.h"

class LocationCard : public DashboardCard {
public:
    float latitude;
    float longitude;
    String label;
    LocationCallback callback;

    LocationCard(const String& id, const String& title, const String& label = "Get Current Location");

    void toJson(JsonObject& card) override;

    void handleAction(const String& action, JsonObject& data) override;

    void setCallback(LocationCallback cb);

    void setLocation(float lat, float lon);
};
