#pragma once

#include <ESPAsyncWebServer.h>
#include <initializer_list>
#include <memory>
#include "dashboard/cards/StatCard.h"
#include "dashboard/cards/StatusCard.h"
#include "dashboard/cards/ChartCard.h"
#include "dashboard/cards/ButtonCard.h"
#include "dashboard/cards/LinkCard.h"
#include "dashboard/cards/TimezoneCard.h"
#include "dashboard/cards/DateCard.h"
#include "dashboard/cards/TimeCard.h"
#include "dashboard/cards/LocationCard.h"
#include "dashboard/cards/ActionButton.h"
#include "dashboard/cards/InputCard.h"
#include "dashboard/cards/ColorPickerCard.h"
#include "dashboard/cards/DropdownCard.h"
#include "dashboard/cards/ToggleCard.h"
#include "dashboard/cards/SliderCard.h"
#include "dashboard/cards/GaugeCard.h"
#include "dashboard/cards/OTACard.h"
#include "dashboard/cards/ConsoleCard.h"

// Public facade. Existing includes, card types and method signatures are retained.
class ESPDashboardPlus {
public:
    using ESPDashboard = ESPDashboardPlus;
    ESPDashboardPlus(const String& title = "ESP32 Dashboard");
    ~ESPDashboardPlus();
    ESPDashboardPlus(const ESPDashboardPlus&) = delete;
    ESPDashboardPlus& operator=(const ESPDashboardPlus&) = delete;
    void begin(AsyncWebServer* server, const uint8_t* htmlData, size_t htmlSize, bool enableOTA = true,
               bool enableConsole = true, const String& wsPath = "/ws");
    void loop();
    void setTitle(const String& title);
    void setSubtitle(const String& subtitle);
    void setTitle(const String& title, const String& subtitle);
    void setVersionInfo(const String& version, const String& lastUpdate = "");
    void onCommand(std::function<void(const String&)> handler);
    bool isOTAEnabled() const;
    bool isConsoleEnabled() const;
    void broadcastUpdate(const String& cardId, JsonObject& data);
    StatCard* addStatCard(const String& id, const String& title, const String& value = "", const String& unit = "");
    StatusCard* addStatusCard(const String& id, const String& title, StatusIcon icon = StatusIcon::INFO);
    ChartCard* addChartCard(const String& id, const String& title, ChartType type = ChartType::LINE,
                            int maxPoints = 20);
    ButtonCardImpl* addButtonCard(const String& id, const String& title, const String& label, ButtonCallback cb);
    LinkCard* addLinkCard(const String& id, const String& title, const String& label, const String& url);
    TimezoneCard* addTimezoneCard(const String& id, const String& title, const String& label = "Get Browser Timezone");
    DateCard* addDateCard(const String& id, const String& title, bool includeTime = false);
    TimeCard* addTimeCard(const String& id, const String& title, bool includeSeconds = false);
    LocationCard* addLocationCard(const String& id, const String& title, const String& label = "Get Current Location");
    ActionButton* addActionButton(const String& id, const String& title, const String& label,
                                  const String& confirmTitle, const String& confirmMsg, ButtonCallback cb);
    InputCard* addInputCard(const String& id, const String& title, const String& placeholder = "");
    ColorPickerCard* addColorPickerCard(const String& id, const String& title, const String& defaultColor = "#00D4AA");
    DropdownCardImpl* addDropdownCard(const String& id, const String& title, const String& placeholder = "Select...");
    ToggleCard* addToggleCard(const String& id, const String& title, const String& label = "",
                              bool defaultValue = false);
    SliderCard* addSliderCard(const String& id, const String& title, int min = 0, int max = 100, int step = 1,
                              const String& unit = "");
    GaugeCard* addGaugeCard(const String& id, const String& title, float min = 0, float max = 100,
                            const String& unit = "%");
    void addGroup(const String& id, const String& title);
    void addCardToGroup(const String& groupId, const String& cardId);
    void addGroup(const String& id, const String& title, std::initializer_list<String> cardIds);
    void removeCardFromGroup(const String& groupId, const String& cardId);
    void removeGroup(const String& id);
    DashboardCard* getCard(const String& id);
    void removeCard(const String& id);
    void updateStatCard(const String& id, const String& value);
    void updateStatusCard(const String& id, StatusIcon icon, CardVariant variant, const String& label,
                          const String& message);
    void updateChartCard(const String& id, float value);
    void updateChartCard(const String& id, int seriesIndex, float value);
    void updateGaugeCard(const String& id, float value);
    void updateToggleCard(const String& id, bool value);
    void updateSliderCard(const String& id, int value);
    void updateColorCard(const String& id, const String& color);
    void updateDropdownCard(const String& id, const String& value);
    void updateDateCard(const String& id, const String& value);
    void updateTimeCard(const String& id, const String& value);
    void updateLocationCard(const String& id, float latitude, float longitude);
    void updateLinkCard(const String& id, const String& url);
    void logDebug(const String& message);
    void logInfo(const String& message);
    void logWarning(const String& message);
    void logError(const String& message);
    void log(LogLevel level, const String& message);

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};
