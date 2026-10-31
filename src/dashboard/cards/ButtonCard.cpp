#include "ButtonCard.h"

ButtonCardImpl::ButtonCardImpl(const String& id, const String& title, const String& label, ButtonCallback cb)
    : DashboardCard(id, CardType::BUTTON, title), label(label), callback(cb) {}

void ButtonCardImpl::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["label"] = label;
    config["variant"] = variantToString(variant);
    if (icon.length() > 0)
        config["icon"] = icon;
    writeSize(config);
}

void ButtonCardImpl::handleAction(const String& action, JsonObject& data) {
    if (action == "click" && callback) {
        callback();
    }
}

void ButtonCardImpl::setIcon(const String& i) {
    icon = i;
}
