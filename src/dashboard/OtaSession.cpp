#include "OtaSession.h"
#include <cstring>
#include <memory>

namespace espdashboardplus {
namespace detail {
namespace {
size_t decodedLength(const String& input) {
    const size_t length = input.length();
    size_t padding = 0;
    if (length >= 2) {
        if (input[length - 1] == '=')
            ++padding;
        if (input[length - 2] == '=')
            ++padding;
    }
    return (length * 3 / 4) - padding;
}

void decode(const String& input, uint8_t* output) {
    static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t written = 0;
    uint32_t buffer = 0;
    int bits = 0;
    for (size_t i = 0; i < input.length(); ++i) {
        const char value = input[i];
        if (value == '=')
            break;
        const char* position = strchr(alphabet, value);
        if (!position)
            continue;
        buffer = (buffer << 6) | (position - alphabet);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            output[written++] = (buffer >> bits) & 0xFF;
        }
    }
}
} // namespace

OtaSession::OtaSession(FirmwareUpdate& firmware) : _firmware(firmware) {}

bool OtaSession::handle(const String& action, JsonObject data) {
    if (action != "ota_start" && action != "ota_chunk" && action != "ota_end")
        return false;
    if (!_firmware.supported()) {
        Serial.println("[Dashboard] OTA not supported on this platform");
        _inProgress = false;
    } else if (action == "ota_start") {
        start(data["size"].as<size_t>());
    } else if (_inProgress && action == "ota_chunk") {
        chunk(data["data"].as<String>());
    } else if (_inProgress && action == "ota_end") {
        finish();
    }
    return true;
}

void OtaSession::start(size_t size) {
    _size = size;
    _received = 0;
    _inProgress = true;
    if (!_firmware.begin(_size)) {
        Serial.print("[Dashboard] OTA begin failed: ");
        _firmware.printError();
        _inProgress = false;
    }
}

void OtaSession::abort() {
    _firmware.abort();
    _inProgress = false;
}

void OtaSession::chunk(const String& encoded) {
    const size_t length = decodedLength(encoded);
    if (length == 0) {
        Serial.println("[Dashboard] OTA chunk: invalid base64 length");
        abort();
        return;
    }
    std::unique_ptr<uint8_t[]> decoded(new uint8_t[length]);
    decode(encoded, decoded.get());
    const size_t written = _firmware.write(decoded.get(), length);
    if (written != length) {
        Serial.printf("[Dashboard] OTA write mismatch: expected %u, wrote %u\n", length, written);
        abort();
        return;
    }
    _received += length;
}

void OtaSession::finish() {
    if (_received != _size) {
        Serial.printf("[Dashboard] OTA size mismatch! Expected %u, received %u\n", _size, _received);
        abort();
        return;
    }
    if (_firmware.end()) {
        Serial.println("[Dashboard] OTA complete, restarting...");
        _firmware.restart(1000);
    } else {
        Serial.println("[Dashboard] OTA end failed");
        _firmware.printError();
    }
    _inProgress = false;
}

} // namespace detail
} // namespace espdashboardplus
