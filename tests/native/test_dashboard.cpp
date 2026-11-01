#include <ESPDashboardPlus.h>
#include <Update.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

void verifyOtaPort();

void check(bool value, const char* description) {
    if (!value) {
        std::cerr << "FAIL: " << description << '\n';
        std::exit(1);
    }
}

void recordCard(std::ostream& out, DashboardCard& card) {
    JsonDocument doc;
    auto object = doc.to<JsonObject>();
    card.toJson(object);
    String message;
    serializeJson(doc, message);
    out << message << '\n';
}

int main(int argc, char** argv) {
    verifyOtaPort();
    std::ostringstream captured;
    AsyncWebServer server(80);
    ESPDashboardPlus dashboard("Test Device");
    dashboard.setSubtitle("Contract");
    dashboard.setVersionInfo("1.2.3", "2026-10-31");
    auto* stat = dashboard.addStatCard("stat", "Temperature", "23.5", "C");
    stat->setVariant(CardVariant::DANGER);
    stat->setWeight(4);
    stat->setSize(2, 3);
    stat->setTrend("up", "+1");
    auto* status = dashboard.addStatusCard("status", "Status");
    status->setStatus(StatusIcon::CHECK, CardVariant::SUCCESS, "OK", "Running");
    auto* chart = dashboard.addChartCard("chart", "History", ChartType::AREA, 2);
    chart->addDataPoint(1);
    chart->addDataPoint(2);
    chart->addDataPoint(3);
    check(chart->data.size() == 2 && chart->data[0] == 2, "bounded single-series chart");
    ChartCard multi("multi", "Multi", ChartType::BAR, 2);
    multi.addSeries("A", "primary");
    multi.addSeries("B", "#123456");
    multi.addDataPoint(0, 1);
    multi.addDataPoint(0, 2);
    multi.addDataPoint(0, 3);
    multi.addDataPoint(1, 4);
    multi.addDataPoint(-1, 5);
    int clicks = 0;
    auto* button = dashboard.addButtonCard("button", "Button", "Go", [&] { ++clicks; });
    button->setIcon("play");
    auto* action = dashboard.addActionButton("action", "Action", "Reset", "Confirm", "Really?", [&] { ++clicks; });
    auto* link = dashboard.addLinkCard("link", "Link", "Docs", "https://example.com");
    link->setTarget("_self");
    auto* timezone = dashboard.addTimezoneCard("timezone", "Timezone");
    auto* date = dashboard.addDateCard("date", "Date", true);
    date->setRange("2026-01-01", "2026-12-31");
    auto* time = dashboard.addTimeCard("time", "Time", true);
    auto* location = dashboard.addLocationCard("location", "Location");
    auto* input = dashboard.addInputCard("input", "Input", "Number");
    input->setNumberInput(1, 10, 0.5, "s");
    auto* color = dashboard.addColorPickerCard("color", "Color");
    auto* dropdown = dashboard.addDropdownCard("dropdown", "Dropdown");
    dropdown->addOption("a", "A");
    dropdown->addOption("b", "B");
    auto* toggle = dashboard.addToggleCard("toggle", "Toggle", "Enabled", false);
    auto* slider = dashboard.addSliderCard("slider", "Slider", 0, 100, 5, "%");
    auto* gauge = dashboard.addGaugeCard("gauge", "Gauge", 0, 100, "%");
    gauge->setThresholds(50, 80);
    ConsoleCard console("console");
    console.addLog(LogLevel::WARNING, "A < B");
    OTACard ota("ota");
    for (auto* card :
         std::vector<DashboardCard*>{stat, status, chart, &multi, button, action, link, timezone, date, time, location,
                                     input, color, dropdown, toggle, slider, gauge, &console, &ota}) {
        recordCard(captured, *card);
    }
    dashboard.addGroup("sensors", "Sensors", {"stat", "chart"});
    dashboard.addGroup("controls", "Controls");
    dashboard.addCardToGroup("controls", "toggle");
    dashboard.removeCardFromGroup("controls", "toggle");
    dashboard.addCardToGroup("controls", "slider");
    dashboard.addGroup("discard", "Discard");
    dashboard.removeGroup("discard");
    int commands = 0;
    dashboard.onCommand([&](const String& cmd) {
        check(cmd == "help", "command text");
        ++commands;
    });
    bool toggleCallback = false;
    toggle->onChange = [&](bool value) { toggleCallback = value; };
    const uint8_t html[] = {31, 139, 0};
    dashboard.begin(&server, html, sizeof(html), true, true, "/custom-ws");
    check(server.ws->path == "/custom-ws", "custom WebSocket path");
    AsyncWebServerRequest configuration;
    server.routes.at("/esp-dashboard/config")(&configuration);
    JsonDocument connectionConfig;
    check(!deserializeJson(connectionConfig, configuration.body), "configuration JSON");
    check(connectionConfig["wsPath"].as<String>() == "/custom-ws", "configured WebSocket path exposed");
    check(configuration.response.headers["Cache-Control"] == "no-store", "configuration is not cached");
    AsyncWebServerRequest request;
    server.route(&request);
    check(request.status == 200 && request.data == html && request.size == sizeof(html), "PROGMEM response");
    check(request.response.headers["Content-Encoding"] == "gzip", "gzip response header");
    check(request.response.headers["Cache-Control"] == "max-age=86400", "cache response header");
    AsyncWebSocketClient client;
    server.ws->connect(client);
    for (const auto& message : client.messages)
        captured << message << '\n';
    client.messages.clear();
    server.ws->receive(client, R"({"type":"init"})", 5);
    check(client.messages.size() == 1, "fragmented init reply");
    captured << client.messages[0] << '\n';
    server.ws->receive(client, R"({"type":"command","command":"help"})");
    server.ws->receive(client, R"({"type":"action","cardId":"toggle","action":"change","data":{"value":true}})");
    server.ws->receive(client, R"({"type":"action","cardId":"button","action":"click","data":{}})");
    server.ws->receive(client, R"({"type":"action","cardId":"action","action":"confirm","data":{}})");
    server.ws->receive(client, "invalid JSON");
    check(commands == 1 && clicks == 2 && toggleCallback && toggle->value, "callbacks dispatched once");
    dashboard.updateStatCard("missing", "bad");
    dashboard.updateStatCard("toggle", "bad");
    dashboard.updateStatCard("stat", "24.5");
    dashboard.updateStatusCard("status", StatusIcon::WARNING, CardVariant::WARNING, "Warm", "Watch");
    dashboard.updateChartCard("chart", 4.0f);
    auto* chart2 = dashboard.addChartCard("chart2", "Multi");
    chart2->addSeries("A");
    dashboard.updateChartCard("chart2", 0, 5.0f);
    dashboard.updateGaugeCard("gauge", 120);
    dashboard.updateToggleCard("toggle", false);
    dashboard.updateSliderCard("slider", 150);
    dashboard.updateColorCard("color", "#abcdef");
    dashboard.updateDropdownCard("dropdown", "b");
    dashboard.updateDateCard("date", "2026-10-31");
    dashboard.updateTimeCard("time", "12:34:56");
    dashboard.updateLocationCard("location", 48.5f, 2.5f);
    dashboard.updateLinkCard("link", "https://example.org");
    dashboard.logDebug("debug");
    dashboard.logInfo("info");
    dashboard.logWarning("warning");
    dashboard.logError("error");
    dashboard.log(LogLevel::INFO, "log");
    dashboard.removeCard("link");
    dashboard.removeCard("missing");
    dashboard.loop();
    for (const auto& message : server.ws->broadcasts)
        captured << message << '\n';
    check(gauge->value == 100 && slider->value == 100, "programmatic values clamped");
    check(dashboard.getCard("link") == nullptr, "removed card unavailable");
    for (bool cardBased : {false, true}) {
        Update = UpdateStub{};
        ESP.restarts = 0;
        testDelay = 0;
        auto sendOta = [&](const String& event, const String& data) {
            String message = cardBased ? "{\"type\":\"action\",\"cardId\":\"stat\",\"action\":\"" + event +
                                             "\",\"data\":" + data + "}"
                                       : "{\"type\":\"" + event + "\"" + (data == "{}" ? "}" : "," + data.substr(1));
            server.ws->receive(client, message);
        };
        sendOta("ota_start", R"({"size":3})");
        sendOta("ota_chunk", R"({"data":"AQID"})");
        sendOta("ota_end", "{}");
        check(Update.bytes == std::vector<uint8_t>({1, 2, 3}), "OTA payload decoded");
        check(Update.ends == 1 && ESP.restarts == 0, "OTA verified before restart");
        JsonDocument acknowledgement;
        check(!deserializeJson(acknowledgement, client.messages.back()), "OTA acknowledgement JSON");
        check(acknowledgement["stage"].as<String>() == "complete", "OTA completion confirmed to browser");
        testMillis += 1000;
        dashboard.loop();
        check(ESP.restarts == 1 && testDelay == 0, "OTA restart deferred to loop");
        Update = UpdateStub{};
        Update.shortWrite = true;
        sendOta("ota_start", R"({"size":3})");
        sendOta("ota_chunk", R"({"data":"AQID"})");
        sendOta("ota_end", "{}");
        check(Update.aborts == 1 && Update.ends == 0, "OTA short write aborts");
        Update = UpdateStub{};
        sendOta("ota_start", R"({"size":4})");
        sendOta("ota_chunk", R"({"data":"AQID"})");
        sendOta("ota_end", "{}");
        check(Update.aborts == 1 && Update.ends == 0, "OTA size mismatch aborts");
        Update = UpdateStub{};
        sendOta("ota_start", R"({"size":3})");
        sendOta("ota_chunk", R"({"data":""})");
        check(Update.aborts == 1, "OTA empty chunk aborts");
        Update = UpdateStub{};
        Update.beginOk = false;
        sendOta("ota_start", R"({"size":3})");
        sendOta("ota_chunk", R"({"data":"AQID"})");
        sendOta("ota_end", "{}");
        check(Update.ends == 0 && Update.bytes.empty(), "OTA failed start ignores chunks");
    }
    const String output = captured.str();
    if (argc == 3 && String(argv[1]) == "--record") {
        std::ofstream(argv[2], std::ios::binary) << output;
    } else {
        check(argc == 2, "expected golden message fixture path");
        std::ifstream file(argv[1], std::ios::binary);
        std::ostringstream expected;
        expected << file.rdbuf();
        check(output == expected.str(), "JSON messages match pre-refactor fixtures");
    }
    std::cout << "Dashboard contracts, callbacks, WebSocket, HTTP and OTA: OK\n";
}
