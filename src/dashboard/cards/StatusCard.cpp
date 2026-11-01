#include "StatusCard.h"

StatusCard::StatusCard(const String& id, const String& title, StatusIcon icon)
    : DashboardCard(id, CardType::STATUS, title), icon(icon) {}

void StatusCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["icon"] = iconToString(icon);
    config["variant"] = variantToString(variant);
    config["label"] = label;
    config["message"] = message;
    writeSize(config);
}

void StatusCard::setStatus(StatusIcon i, CardVariant v, const String& lbl, const String& msg) {
    icon = i;
    variant = v;
    label = lbl;
    message = msg;
}

void StatusCard::setLabel(const String& lbl) {
    label = lbl;
}

void StatusCard::setMessage(const String& msg) {
    message = msg;
}

void StatusCard::setIcon(StatusIcon i) {
    icon = i;
}
