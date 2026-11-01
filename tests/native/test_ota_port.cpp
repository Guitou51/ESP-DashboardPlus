#include "dashboard/OtaSession.h"
#include <cstdlib>
#include <iostream>
#include <vector>

namespace {
class MemoryFirmware : public espdashboardplus::detail::FirmwareUpdate {
public:
    bool available = true;
    bool result = true;
    int starts = 0, finishes = 0, aborts = 0, restarts = 0, errors = 0;
    size_t expected = 0;
    std::vector<uint8_t> bytes;
    bool supported() const override {
        return available;
    }
    bool begin(size_t size) override {
        ++starts;
        expected = size;
        bytes.clear();
        return true;
    }
    size_t write(uint8_t* data, size_t size) override {
        bytes.insert(bytes.end(), data, data + size);
        return size;
    }
    bool end() override {
        ++finishes;
        return result;
    }
    void abort() override {
        ++aborts;
    }
    void printError() override {
        ++errors;
    }
    void restart(unsigned long) override {
        ++restarts;
    }
};
} // namespace

void verifyOtaPort() {
    MemoryFirmware memory;
    espdashboardplus::detail::OtaSession session(memory);
    auto send = [&](const String& action, const char* json) {
        JsonDocument doc;
        if (deserializeJson(doc, json))
            std::abort();
        session.handle(action, doc.as<JsonObject>());
    };
    send("ota_start", R"({"size":3})");
    send("ota_chunk", R"({"data":"AQI="})");
    send("ota_chunk", R"({"data":"Aw=="})");
    send("ota_end", "{}");
    if (memory.bytes != std::vector<uint8_t>({1, 2, 3}) || memory.restarts != 0 || memory.expected != 3) {
        std::cerr << "FAIL: injected OTA backend and padded chunks\n";
        std::abort();
    }
    session.poll(millis() + 999);
    if (memory.restarts != 0)
        std::abort();
    session.poll(millis() + 1000);
    if (memory.restarts != 1)
        std::abort();
    memory.result = false;
    send("ota_start", R"({"size":1})");
    send("ota_chunk", R"({"data":"AA=="})");
    send("ota_end", "{}");
    if (memory.finishes != 2 || memory.restarts != 1 || memory.errors != 1)
        std::abort();
    memory.available = false;
    send("ota_start", R"({"size":1})");
    send("ota_chunk", R"({"data":"AA=="})");
    send("ota_end", "{}");
    if (memory.starts != 2 || memory.finishes != 2)
        std::abort();

    memory.available = true;
    memory.result = true;
    std::vector<String> acknowledgements;
    auto request = [&](const String& action, const char* json, uint32_t client = 7) {
        JsonDocument doc;
        if (deserializeJson(doc, json))
            std::abort();
        session.handle(
            action, doc.as<JsonObject>(), [&](const String& message) { acknowledgements.push_back(message); }, client);
        JsonDocument result;
        if (deserializeJson(result, acknowledgements.back()))
            std::abort();
        if (result["type"].as<String>() != "ota_status" || result["action"].as<String>() != action ||
            result["uploadId"].as<String>() != doc["uploadId"].as<String>() ||
            result["seq"].as<size_t>() != doc["seq"].as<size_t>())
            std::abort();
        if (result["stage"].as<String>() == "progress" && result["received"].as<size_t>() != memory.bytes.size())
            std::abort();
        return result["stage"].as<String>();
    };
    if (request("ota_start", R"({"uploadId":"one","seq":0,"size":3})") != "ready")
        std::abort();
    if (request("ota_start", R"({"uploadId":"two","seq":0,"size":3})", 8) != "error")
        std::abort();
    session.disconnect(8);
    if (request("ota_chunk", R"({"uploadId":"one","seq":1,"data":"AQID"})", 8) != "error")
        std::abort();
    if (!memory.bytes.empty())
        std::abort();
    if (request("ota_chunk", R"({"uploadId":"one","seq":1,"data":"AQID"})") != "progress")
        std::abort();
    if (request("ota_end", R"({"uploadId":"one","seq":2})") != "complete")
        std::abort();
    if (memory.restarts != 1)
        std::abort();
    session.poll(millis() + 1000);
    if (memory.restarts != 2)
        std::abort();
    if (request("ota_start", R"({"uploadId":"one","seq":0,"size":3})") != "ready")
        std::abort();
    if (request("ota_chunk", R"({"uploadId":"one","seq":2,"data":"AQID"})") != "error")
        std::abort();
    for (const char* invalid : {"====", "A===", "A?ID", "AQI", ""}) {
        JsonDocument doc;
        request("ota_start", R"({"uploadId":"one","seq":0,"size":3})");
        doc["uploadId"] = "one";
        doc["seq"] = 1;
        doc["data"] = invalid;
        session.handle(
            "ota_chunk", doc.as<JsonObject>(), [&](const String& message) { acknowledgements.push_back(message); }, 7);
        JsonDocument response;
        if (deserializeJson(response, acknowledgements.back()) || response["stage"].as<String>() != "error")
            std::abort();
    }
    request("ota_start", R"({"uploadId":"one","seq":0,"size":3})");
    if (request("ota_chunk", R"({"uploadId":"one","seq":1,"data":"AQIDBA=="})") != "error")
        std::abort();
    request("ota_start", R"({"uploadId":"one","seq":0,"size":3})");
    if (request("ota_abort", R"({"uploadId":"one","seq":1})") != "aborted")
        std::abort();
    request("ota_start", R"({"uploadId":"one","seq":0,"size":3})");
    session.disconnect(7);
    if (request("ota_chunk", R"({"uploadId":"one","seq":1,"data":"AQID"})") != "error")
        std::abort();
    request("ota_start", R"({"uploadId":"one","seq":0,"size":3})");
    session.poll(millis() + 30000);
    if (request("ota_end", R"({"uploadId":"one","seq":1})") != "error")
        std::abort();
    if (request("ota_start", R"({"uploadId":"one","seq":0,"size":0})") != "error")
        std::abort();
    JsonDocument start;
    start["uploadId"] = "callback";
    start["seq"] = 0;
    start["size"] = 3;
    bool callbackReturned = false;
    session.handle(
        "ota_start", start.as<JsonObject>(),
        [&](const String&) {
            session.disconnect(7);
            callbackReturned = true;
        },
        7);
    if (!callbackReturned)
        std::abort();
    if (request("ota_chunk", R"({"uploadId":"callback","seq":1,"data":"AQID"})") != "error")
        std::abort();
}
