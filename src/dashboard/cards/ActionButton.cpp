#include "ActionButton.h"

ActionButton::ActionButton(const String& id, const String& title, const String& label, const String& confirmTitle,
                           const String& confirmMsg, ButtonCallback cb)
    : DashboardCard(id, CardType::ACTION, title), label(label), confirmTitle(confirmTitle), confirmMessage(confirmMsg),
      callback(cb) {
    variant = CardVariant::WARNING;
}

void ActionButton::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["label"] = label;
    config["confirmTitle"] = confirmTitle;
    config["confirmMessage"] = confirmMessage;
    config["variant"] = variantToString(variant);
    if (icon.length() > 0)
        config["icon"] = icon;
    writeSize(config);
}

void ActionButton::handleAction(const String& action, JsonObject& data) {
    if (action == "confirm" && callback) {
        callback();
    }
}

void ActionButton::setIcon(const String& i) {
    icon = i;
}
