#include "DashboardCard.h"

DashboardCard::DashboardCard(const String& id, CardType type, const String& title)
    : id(id), type(type), title(title), variant(CardVariant::PRIMARY), weight(0), sizeX(1), sizeY(1) {}

DashboardCard::~DashboardCard() = default;

void DashboardCard::handleAction(const String& action, JsonObject& data) {}

void DashboardCard::setVariant(CardVariant v) {
    variant = v;
}

void DashboardCard::setWeight(int w) {
    weight = w;
}

int DashboardCard::getWeight() const {
    return weight;
}

void DashboardCard::setSize(int x, int y) {
    sizeX = x;
    sizeY = y;
}

void DashboardCard::setSizeX(int x) {
    sizeX = x;
}

void DashboardCard::setSizeY(int y) {
    sizeY = y;
}

String DashboardCard::getVariantString() {
    return variantToString(variant);
}

String DashboardCard::getIconString(StatusIcon i) {
    return iconToString(i);
}

String DashboardCard::variantToString(CardVariant v) {
    return espdashboardplus::detail::variantName(v);
}

String DashboardCard::typeToString(CardType t) {
    return espdashboardplus::detail::cardTypeName(t);
}

String DashboardCard::logLevelToString(LogLevel level) {
    return espdashboardplus::detail::logLevelName(level);
}

String DashboardCard::chartTypeToString(ChartType t) {
    return espdashboardplus::detail::chartTypeName(t);
}

String DashboardCard::iconToString(StatusIcon i) {
    return espdashboardplus::detail::iconName(i);
}

JsonObject DashboardCard::beginJson(JsonObject& card) {
    card["id"] = id;
    card["type"] = typeToString(type);
    card["weight"] = weight;
    JsonObject config = card["config"].to<JsonObject>();
    config["title"] = title;
    return config;
}

void DashboardCard::writeSize(JsonObject& config) {
    if (sizeX > 1)
        config["sizeX"] = sizeX;
    if (sizeY > 1)
        config["sizeY"] = sizeY;
}
