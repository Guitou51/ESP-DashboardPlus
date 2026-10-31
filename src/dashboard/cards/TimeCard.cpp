#include "TimeCard.h"

TimeCard::TimeCard(const String& id, const String& title, bool includeSeconds)
    : DashboardCard(id, CardType::TIME, title), includeSeconds(includeSeconds) {}

void TimeCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    config["includeSeconds"] = includeSeconds;
    writeSize(config);
}

void TimeCard::handleAction(const String& action, JsonObject& data) {
    if (action == "change" && !data["value"].isNull()) {
        value = data["value"].as<String>();
        if (callback)
            callback(value);
    }
}

void TimeCard::setCallback(TimeCallback cb) {
    callback = cb;
}

void TimeCard::setValue(const String& val) {
    value = val;
}
