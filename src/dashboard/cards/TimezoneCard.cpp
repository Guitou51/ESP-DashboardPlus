#include "TimezoneCard.h"

TimezoneCard::TimezoneCard(const String& id, const String& title, const String& label)
    : DashboardCard(id, CardType::TIMEZONE, title), label(label) {
    variant = CardVariant::INFO;
}

void TimezoneCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["label"] = label;
    config["value"] = value;
    config["variant"] = variantToString(variant);
    writeSize(config);
}

void TimezoneCard::handleAction(const String& action, JsonObject& data) {
    if (action == "timezone" && callback) {
        String tz = data["timezone"].as<String>();
        int offset = data["offset"].as<int>();
        String offsetStr = data["offsetString"].as<String>();
        value = tz + " (" + offsetStr + ")";
        callback(tz, offset, offsetStr);
    }
}

void TimezoneCard::setCallback(TimezoneCallback cb) {
    callback = cb;
}
