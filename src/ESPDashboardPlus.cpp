#include "ESPDashboardPlus.h"
#include "dashboard/AsyncDashboardTransport.h"
#include "dashboard/CardRegistry.h"
#include "dashboard/FirmwareUpdate.h"
#include "dashboard/JsonProtocol.h"
#include "dashboard/MessageRouter.h"
#include "dashboard/OtaSession.h"
#include <utility>

using namespace espdashboardplus::detail;

class ESPDashboardPlus::Impl {
public:
    DashboardState state;
    CardRegistry cards;
    ArduinoFirmwareUpdate firmware;
    OtaSession ota;
    MessageRouter router;
    AsyncDashboardTransport transport;

    explicit Impl(const String& title) : ota(firmware), router(cards, ota, state) {
        state.title = title;
    }

    template <class Card, class... Args> Card* add(Args&&... args) {
        auto* card = new Card(std::forward<Args>(args)...);
        cards.add(card);
        return card;
    }

    template <class Card, class Mutate> void update(const String& id, CardType type, Mutate mutate) {
        auto* base = cards.get(id);
        if (!base || base->type != type)
            return;
        JsonDocument patch;
        auto data = patch.to<JsonObject>();
        mutate(static_cast<Card*>(base), data);
        if (transport.ready())
            transport.broadcast(updateMessage(id, data));
    }
};

ESPDashboardPlus::ESPDashboardPlus(const String& title) : _impl(new Impl(title)) {}
ESPDashboardPlus::~ESPDashboardPlus() = default;

void ESPDashboardPlus::begin(AsyncWebServer* server, const uint8_t* htmlData, size_t htmlSize, bool enableOTA,
                             bool enableConsole, const String& wsPath) {
    _impl->state.enableOTA = enableOTA;
    _impl->state.enableConsole = enableConsole;
    _impl->transport.begin(
        server, htmlData, htmlSize, wsPath, [this](const MessageReply& reply) { _impl->router.initialize(reply); },
        [this](const uint8_t* data, size_t size, const MessageReply& reply) {
            _impl->router.handle(data, size, reply);
        });
    Serial.println("[Dashboard] Initialized with PROGMEM HTML");
    Serial.printf("[Dashboard] OTA: %s, Console: %s\n", enableOTA ? "enabled" : "disabled",
                  enableConsole ? "enabled" : "disabled");
}

void ESPDashboardPlus::loop() {
    _impl->transport.loop();
}

void ESPDashboardPlus::setTitle(const String& title) {
    _impl->state.title = title;
}

void ESPDashboardPlus::setSubtitle(const String& subtitle) {
    _impl->state.subtitle = subtitle;
}

void ESPDashboardPlus::setTitle(const String& title, const String& subtitle) {
    _impl->state.title = title;
    _impl->state.subtitle = subtitle;
}

void ESPDashboardPlus::setVersionInfo(const String& version, const String& lastUpdate) {
    _impl->state.version = version;
    _impl->state.lastUpdate = lastUpdate;
}

void ESPDashboardPlus::onCommand(std::function<void(const String&)> handler) {
    _impl->state.onCommand = handler;
}

bool ESPDashboardPlus::isOTAEnabled() const {
    return _impl->state.enableOTA;
}

bool ESPDashboardPlus::isConsoleEnabled() const {
    return _impl->state.enableConsole;
}

void ESPDashboardPlus::broadcastUpdate(const String& cardId, JsonObject& data) {
    if (_impl->transport.ready())
        _impl->transport.broadcast(updateMessage(cardId, data));
}

StatCard* ESPDashboardPlus::addStatCard(const String& id, const String& title, const String& value,
                                        const String& unit) {
    return _impl->add<StatCard>(id, title, value, unit);
}

StatusCard* ESPDashboardPlus::addStatusCard(const String& id, const String& title, StatusIcon icon) {
    return _impl->add<StatusCard>(id, title, icon);
}

ChartCard* ESPDashboardPlus::addChartCard(const String& id, const String& title, ChartType type, int maxPoints) {
    return _impl->add<ChartCard>(id, title, type, maxPoints);
}

ButtonCardImpl* ESPDashboardPlus::addButtonCard(const String& id, const String& title, const String& label,
                                                ButtonCallback cb) {
    return _impl->add<ButtonCardImpl>(id, title, label, cb);
}

LinkCard* ESPDashboardPlus::addLinkCard(const String& id, const String& title, const String& label, const String& url) {
    return _impl->add<LinkCard>(id, title, label, url);
}

TimezoneCard* ESPDashboardPlus::addTimezoneCard(const String& id, const String& title, const String& label) {
    return _impl->add<TimezoneCard>(id, title, label);
}

DateCard* ESPDashboardPlus::addDateCard(const String& id, const String& title, bool includeTime) {
    return _impl->add<DateCard>(id, title, includeTime);
}

TimeCard* ESPDashboardPlus::addTimeCard(const String& id, const String& title, bool includeSeconds) {
    return _impl->add<TimeCard>(id, title, includeSeconds);
}

LocationCard* ESPDashboardPlus::addLocationCard(const String& id, const String& title, const String& label) {
    return _impl->add<LocationCard>(id, title, label);
}

ActionButton* ESPDashboardPlus::addActionButton(const String& id, const String& title, const String& label,
                                                const String& confirmTitle, const String& confirmMsg,
                                                ButtonCallback cb) {
    return _impl->add<ActionButton>(id, title, label, confirmTitle, confirmMsg, cb);
}

InputCard* ESPDashboardPlus::addInputCard(const String& id, const String& title, const String& placeholder) {
    return _impl->add<InputCard>(id, title, placeholder);
}

ColorPickerCard* ESPDashboardPlus::addColorPickerCard(const String& id, const String& title,
                                                      const String& defaultColor) {
    return _impl->add<ColorPickerCard>(id, title, defaultColor);
}

DropdownCardImpl* ESPDashboardPlus::addDropdownCard(const String& id, const String& title, const String& placeholder) {
    return _impl->add<DropdownCardImpl>(id, title, placeholder);
}

ToggleCard* ESPDashboardPlus::addToggleCard(const String& id, const String& title, const String& label,
                                            bool defaultValue) {
    return _impl->add<ToggleCard>(id, title, label, defaultValue);
}

SliderCard* ESPDashboardPlus::addSliderCard(const String& id, const String& title, int min, int max, int step,
                                            const String& unit) {
    return _impl->add<SliderCard>(id, title, min, max, step, unit);
}

GaugeCard* ESPDashboardPlus::addGaugeCard(const String& id, const String& title, float min, float max,
                                          const String& unit) {
    return _impl->add<GaugeCard>(id, title, min, max, unit);
}

void ESPDashboardPlus::addGroup(const String& id, const String& title) {
    _impl->cards.addGroup(id, title);
}

void ESPDashboardPlus::addCardToGroup(const String& groupId, const String& cardId) {
    _impl->cards.addCardToGroup(groupId, cardId);
}

void ESPDashboardPlus::addGroup(const String& id, const String& title, std::initializer_list<String> cardIds) {
    _impl->cards.addGroup(id, title, cardIds);
}

void ESPDashboardPlus::removeCardFromGroup(const String& groupId, const String& cardId) {
    _impl->cards.removeCardFromGroup(groupId, cardId);
}

void ESPDashboardPlus::removeGroup(const String& id) {
    _impl->cards.removeGroup(id);
}

DashboardCard* ESPDashboardPlus::getCard(const String& id) {
    return _impl->cards.get(id);
}

void ESPDashboardPlus::removeCard(const String& id) {
    if (_impl->cards.remove(id))
        _impl->transport.broadcast(removeMessage(id));
}

void ESPDashboardPlus::updateStatCard(const String& id, const String& value) {
    _impl->update<StatCard>(id, CardType::STAT, [&](auto* card, JsonObject& data) {
        card->setValue(value);

        data["value"] = value;
    });
}

void ESPDashboardPlus::updateStatusCard(const String& id, StatusIcon icon, CardVariant variant, const String& label,
                                        const String& message) {
    _impl->update<StatusCard>(id, CardType::STATUS, [&](auto* card, JsonObject& data) {
        card->setStatus(icon, variant, label, message);

        data["icon"] = card->getIconString(icon);
        data["variant"] = card->getVariantString();
        data["label"] = label;
        data["message"] = message;
    });
}

void ESPDashboardPlus::updateChartCard(const String& id, float value) {
    _impl->update<ChartCard>(id, CardType::CHART, [&](auto* card, JsonObject& data) {
        card->addDataPoint(value);

        JsonArray dataArr = data["data"].to<JsonArray>();
        for (float v : card->data) {
            dataArr.add(v);
        }
    });
}

void ESPDashboardPlus::updateChartCard(const String& id, int seriesIndex, float value) {
    _impl->update<ChartCard>(id, CardType::CHART, [&](auto* card, JsonObject& data) {
        card->addDataPoint(seriesIndex, value);

        JsonArray seriesArr = data["series"].to<JsonArray>();
        for (const ChartSeries& s : card->series) {
            JsonObject seriesObj = seriesArr.add<JsonObject>();
            seriesObj["name"] = s.name;
            seriesObj["color"] = s.color;
            JsonArray dataArr = seriesObj["data"].to<JsonArray>();
            for (float v : s.data) {
                dataArr.add(v);
            }
        }
    });
}

void ESPDashboardPlus::updateGaugeCard(const String& id, float value) {
    _impl->update<GaugeCard>(id, CardType::GAUGE, [&](auto* card, JsonObject& data) {
        card->setValue(value);

        data["value"] = value;
    });
}

void ESPDashboardPlus::updateToggleCard(const String& id, bool value) {
    _impl->update<ToggleCard>(id, CardType::TOGGLE, [&](auto* card, JsonObject& data) {
        card->setValue(value);

        data["value"] = value;
    });
}

void ESPDashboardPlus::updateSliderCard(const String& id, int value) {
    _impl->update<SliderCard>(id, CardType::SLIDER, [&](auto* card, JsonObject& data) {
        card->setValue(value);

        data["value"] = value;
    });
}

void ESPDashboardPlus::updateColorCard(const String& id, const String& color) {
    _impl->update<ColorPickerCard>(id, CardType::COLOR, [&](auto* card, JsonObject& data) {
        card->setValue(color);

        data["value"] = color;
    });
}

void ESPDashboardPlus::updateDropdownCard(const String& id, const String& value) {
    _impl->update<DropdownCardImpl>(id, CardType::DROPDOWN, [&](auto* card, JsonObject& data) {
        card->setValue(value);

        data["value"] = value;
    });
}

void ESPDashboardPlus::updateDateCard(const String& id, const String& value) {
    _impl->update<DateCard>(id, CardType::DATE, [&](auto* card, JsonObject& data) {
        card->setValue(value);

        data["value"] = value;
    });
}

void ESPDashboardPlus::updateTimeCard(const String& id, const String& value) {
    _impl->update<TimeCard>(id, CardType::TIME, [&](auto* card, JsonObject& data) {
        card->setValue(value);

        data["value"] = value;
    });
}

void ESPDashboardPlus::updateLocationCard(const String& id, float latitude, float longitude) {
    _impl->update<LocationCard>(id, CardType::LOCATION, [&](auto* card, JsonObject& data) {
        card->setLocation(latitude, longitude);

        data["latitude"] = latitude;
        data["longitude"] = longitude;
    });
}

void ESPDashboardPlus::updateLinkCard(const String& id, const String& url) {
    _impl->update<LinkCard>(id, CardType::LINK, [&](auto* card, JsonObject& data) {
        card->setUrl(url);

        data["url"] = url;
    });
}

void ESPDashboardPlus::logDebug(const String& message) {
    log(LogLevel::DEBUG, message);
}

void ESPDashboardPlus::logInfo(const String& message) {
    log(LogLevel::INFO, message);
}

void ESPDashboardPlus::logWarning(const String& message) {
    log(LogLevel::WARNING, message);
}

void ESPDashboardPlus::logError(const String& message) {
    log(LogLevel::ERROR, message);
}

void ESPDashboardPlus::log(LogLevel level, const String& message) {
    if (_impl->transport.ready() && _impl->state.enableConsole) {
        _impl->transport.broadcast(logMessage(level, message, millis()));
    }
}
