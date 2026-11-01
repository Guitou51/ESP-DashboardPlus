#include "LinkCard.h"

LinkCard::LinkCard(const String& id, const String& title, const String& label, const String& url)
    : DashboardCard(id, CardType::LINK, title), label(label), url(url), target("_blank") {}

void LinkCard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["label"] = label;
    config["url"] = url;
    config["target"] = target;
    config["variant"] = variantToString(variant);
    if (icon.length() > 0)
        config["icon"] = icon;
    writeSize(config);
}

void LinkCard::setTarget(const String& t) {
    target = t;
}

void LinkCard::setIcon(const String& i) {
    icon = i;
}

void LinkCard::setUrl(const String& u) {
    url = u;
}
