#include "AsyncDashboardTransport.h"
#include <ESPAsyncWebServer.h>
#include <cstring>

namespace espdashboardplus {
namespace detail {

AsyncDashboardTransport::AsyncDashboardTransport()
    : _socket(nullptr), _html(nullptr), _htmlSize(0), _lastHeartbeat(0), _bufferSize(0), _bufferFilled(0) {}

AsyncDashboardTransport::~AsyncDashboardTransport() = default;

void AsyncDashboardTransport::begin(AsyncWebServer* server, const uint8_t* html, size_t size, const String& path,
                                    Connected connected, Received received, Disconnected disconnected) {
    _socket = new AsyncWebSocket(path);
    _html = html;
    _htmlSize = size;
    _socket->onEvent([this, connected, received, disconnected](AsyncWebSocket*, AsyncWebSocketClient* client,
                                                               AwsEventType type, void* arg, uint8_t* data,
                                                               size_t length) {
        const MessageReply reply = [client](const String& message) { client->text(message); };
        if (type == WS_EVT_CONNECT) {
            Serial.printf("[Dashboard] Client #%u connected\n", client->id());
            connected(reply);
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("[Dashboard] Client #%u disconnected\n", client->id());
            if (disconnected)
                disconnected(client->id());
        } else if (type == WS_EVT_DATA) {
            const auto* info = static_cast<AwsFrameInfo*>(arg);
            if (info->opcode != WS_TEXT)
                return;
            if (info->final && info->index == 0 && info->len == length) {
                received(data, length, reply, client->id());
                return;
            }
            if (info->index == 0) {
                _buffer.reset();
                _bufferSize = info->len;
                _bufferFilled = 0;
                _buffer.reset(new uint8_t[_bufferSize]);
            }
            if (_buffer && info->index + length <= _bufferSize) {
                memcpy(_buffer.get() + info->index, data, length);
                _bufferFilled += length;
            }
            if (info->final && _buffer && _bufferFilled == _bufferSize) {
                received(_buffer.get(), _bufferSize, reply, client->id());
                _buffer.reset();
                _bufferSize = _bufferFilled = 0;
            }
        }
    });
    server->addHandler(_socket);
    server->on("/esp-dashboard/config", HTTP_GET, [path](AsyncWebServerRequest* request) {
        JsonDocument doc;
        doc["wsPath"] = path;
        String body;
        serializeJson(doc, body);
        auto* response = request->beginResponse(200, "application/json", body);
        response->addHeader("Cache-Control", "no-store");
        request->send(response);
    });
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        auto* response = request->beginResponse(200, "text/html", _html, _htmlSize);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "max-age=86400");
        request->send(response);
    });
}

bool AsyncDashboardTransport::ready() const {
    return _socket != nullptr;
}

void AsyncDashboardTransport::broadcast(const String& message) {
    if (_socket)
        _socket->textAll(message);
}

void AsyncDashboardTransport::loop() {
    if (!_socket)
        return;
    _socket->cleanupClients();
    const auto now = millis();
    if (now - _lastHeartbeat >= 2500) {
        broadcast(heartbeatMessage(millis()));
        _lastHeartbeat = now;
    }
}

} // namespace detail
} // namespace espdashboardplus
