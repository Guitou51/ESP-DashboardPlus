#pragma once

#include "Types.h"
#include <ArduinoJson.h>

class DashboardCard {
public:
    String id;
    CardType type;
    String title;
    CardVariant variant;
    int weight; // Lower weight = shown first (default: 0)
    int sizeX;  // Grid column span (default: 1)
    int sizeY;  // Grid row span (default: 1)

    DashboardCard(const String& id, CardType type, const String& title);

    virtual ~DashboardCard();

    virtual void toJson(JsonObject& card) = 0;
    virtual void handleAction(const String& action, JsonObject& data);

    void setVariant(CardVariant v);

    void setWeight(int w);

    int getWeight() const;

    void setSize(int x, int y);

    void setSizeX(int x);

    void setSizeY(int y);

    // Public access for update helpers
    String getVariantString();

    String getIconString(StatusIcon i);

protected:
    JsonObject beginJson(JsonObject& card);
    void writeSize(JsonObject& config);
    String variantToString(CardVariant v);

    String typeToString(CardType t);

    String logLevelToString(LogLevel level);

    String chartTypeToString(ChartType t);

    String iconToString(StatusIcon i);
};
