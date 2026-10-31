#pragma once

#include "../DashboardCard.h"

class OTACard : public DashboardCard {
public:
    int maxSizeMB;
    std::function<void(size_t, size_t)> onProgress;
    std::function<void(bool)> onComplete;

    OTACard(const String& id, const String& title = "Firmware Update (OTA)", int maxSize = 4);

    void toJson(JsonObject& card) override;
};
