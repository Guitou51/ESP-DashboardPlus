#include "DateCard.h"

DateCard::DateCard(const String& id, const String& title, bool includeTime)
    : DashboardCard(id, CardType::DATE, title), includeTime(includeTime) {}

void DateCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["value"] = value;
    config["includeTime"] = includeTime;
    if (minDate.length() > 0)
        config["min"] = minDate;
    if (maxDate.length() > 0)
        config["max"] = maxDate;
    writeSize(config);
}

void DateCard::handleAction(const String& action, JsonObject& data) {
    if (action == "change" && !data["value"].isNull()) {
        value = data["value"].as<String>();
        if (callback)
            callback(value);
    }
}

void DateCard::setCallback(DateCallback cb) {
    callback = cb;
}

void DateCard::setRange(const String& min, const String& max) {
    minDate = min;
    maxDate = max;
}

void DateCard::setValue(const String& val) {
    value = val;
}
