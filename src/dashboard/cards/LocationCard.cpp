#include "LocationCard.h"

LocationCard::LocationCard(const String& id, const String& title, const String& label)
    : DashboardCard(id, CardType::LOCATION, title), label(label), latitude(0), longitude(0) {
    variant = CardVariant::INFO;
}

void LocationCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["label"] = label;
    config["latitude"] = latitude;
    config["longitude"] = longitude;
    config["variant"] = variantToString(variant);
    writeSize(config);
}

void LocationCard::handleAction(const String& action, JsonObject& data) {
    if (action == "location") {
        latitude = data["latitude"].as<float>();
        longitude = data["longitude"].as<float>();
        if (callback)
            callback(latitude, longitude);
    }
}

void LocationCard::setCallback(LocationCallback cb) {
    callback = cb;
}

void LocationCard::setLocation(float lat, float lon) {
    latitude = lat;
    longitude = lon;
}
