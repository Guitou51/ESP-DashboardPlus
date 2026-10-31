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
    if (memory.bytes != std::vector<uint8_t>({1, 2, 3}) || memory.restarts != 1 || memory.expected != 3) {
        std::cerr << "FAIL: injected OTA backend and padded chunks\n";
        std::abort();
    }
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
}
