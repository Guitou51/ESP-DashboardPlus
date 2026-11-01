#include "OtaSession.h"
#include <cstring>
#include <memory>

namespace espdashboardplus {
namespace detail {
namespace {
size_t decodedLength(const String& input) {
    const size_t length = input.length();
    if (length == 0 || length > 4096 || length % 4 != 0)
        return 0;
    size_t padding = 0;
    if (input[length - 1] == '=')
        ++padding;
    if (input[length - 2] == '=')
        ++padding;
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (size_t i = 0; i < length; ++i) {
        if (i >= length - padding) {
            if (input[i] != '=')
                return 0;
        } else if (input[i] == '\0' || !strchr(alphabet, input[i])) {
            return 0;
        }
    }
    return (length / 4) * 3 - padding;
}

void decode(const String& input, uint8_t* output) {
    const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    uint32_t buffer = 0;
    int bits = 0;
    size_t written = 0;
    for (size_t i = 0; i < input.length() && input[i] != '='; ++i) {
        buffer = (buffer << 6) | (strchr(alphabet, input[i]) - alphabet);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            output[written++] = (buffer >> bits) & 0xFF;
        }
    }
}
} // namespace

OtaSession::OtaSession(FirmwareUpdate& firmware) : _firmware(firmware) {}

String OtaSession::statusMessage(const String& action, const String& uploadId, size_t sequence, const String& stage,
                                 const String& error) const {
    JsonDocument doc;
    doc["type"] = "ota_status";
    doc["action"] = action;
    doc["uploadId"] = uploadId;
    doc["seq"] = sequence;
    doc["stage"] = stage;
    doc["received"] = _received;
    doc["size"] = _size;
    if (error.length())
        doc["error"] = error;
    String message;
    serializeJson(doc, message);
    return message;
}

bool OtaSession::handle(const String& action, JsonObject data, const OtaReply& reply, uint32_t clientId) {
    if (action != "ota_start" && action != "ota_chunk" && action != "ota_end" && action != "ota_abort")
        return false;
    std::unique_lock<std::mutex> lock(_mutex);
    const String uploadId = data["uploadId"].is<const char*>() ? data["uploadId"].as<String>() : String();
    const size_t sequence = data["seq"].as<size_t>();
    String error;
    String stage = "error";
    if (!_firmware.supported()) {
        error = "OTA is not supported on this device";
    } else if (action == "ota_start") {
        if (_inProgress || _restartPending) {
            error = "Another upload or restart is already in progress";
        } else {
            error = start(data["size"].as<size_t>());
            if (!error.length()) {
                _owner = clientId;
                _uploadId = uploadId;
                _nextSequence = sequence + 1;
                _lastActivity = millis();
                stage = "ready";
            }
        }
    } else if (!_inProgress) {
        error = "No upload is in progress";
    } else if (clientId != _owner || uploadId != _uploadId) {
        error = "This upload belongs to another connection";
    } else if (action == "ota_abort") {
        abort();
        stage = "aborted";
    } else if (_uploadId.length() && sequence != _nextSequence) {
        error = "Unexpected OTA message sequence";
        abort();
    } else {
        error = action == "ota_chunk" ? chunk(data["data"].as<String>()) : finish();
        if (!error.length()) {
            ++_nextSequence;
            _lastActivity = millis();
            stage = action == "ota_chunk" ? "progress" : "complete";
        }
    }
    const String response = reply ? statusMessage(action, uploadId, sequence, stage, error) : String();
    lock.unlock();
    if (reply)
        reply(response);
    return true;
}

String OtaSession::start(size_t size) {
    _size = size;
    _received = 0;
    if (size == 0)
        return "The firmware file is empty";
    if (!_firmware.begin(size)) {
        _firmware.printError();
        return "Cannot start OTA: check firmware size and OTA partition";
    }
    _inProgress = true;
    return "";
}

void OtaSession::abort() {
    if (_inProgress)
        _firmware.abort();
    _inProgress = false;
}

String OtaSession::chunk(const String& encoded) {
    const size_t length = decodedLength(encoded);
    if (length == 0 || length > _size - _received) {
        abort();
        return "Invalid OTA chunk or firmware size exceeded";
    }
    std::unique_ptr<uint8_t[]> decoded(new uint8_t[length]);
    decode(encoded, decoded.get());
    if (_firmware.write(decoded.get(), length) != length) {
        abort();
        return "Writing the firmware failed";
    }
    _received += length;
    return "";
}

String OtaSession::finish() {
    if (_received != _size) {
        abort();
        return "Incomplete firmware upload";
    }
    _inProgress = false;
    if (!_firmware.end()) {
        _firmware.printError();
        return "Firmware verification failed";
    }
    // Return from the WebSocket callback so its confirmation can be transmitted.
    _restartPending = true;
    _completedAt = millis();
    return "";
}

void OtaSession::disconnect(uint32_t clientId) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_inProgress && _owner == clientId)
        abort();
}

void OtaSession::poll(unsigned long now) {
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    if (!lock.owns_lock())
        return;
    if (_inProgress && now - _lastActivity >= 30000UL)
        abort();
    if (_restartPending && now - _completedAt >= 1000UL) {
        _restartPending = false;
        _firmware.restart(0);
    }
}

} // namespace detail
} // namespace espdashboardplus
