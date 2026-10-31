#include "OTACard.h"

OTACard::OTACard(const String& id, const String& title, int maxSize)
    : DashboardCard(id, CardType::OTA, title), maxSizeMB(maxSize) {}

void OTACard::toJson(JsonObject& card) {
    JsonObject config = beginJson(card);
    config["maxSize"] = maxSizeMB;
}
