#pragma once
#include <Arduino.h>
#include <functional>
#include <map>
#include <vector>

enum AwsEventType { WS_EVT_CONNECT, WS_EVT_DISCONNECT, WS_EVT_DATA };
constexpr int WS_TEXT = 1;
constexpr int HTTP_GET = 1;
struct AwsFrameInfo {
    bool final;
    size_t index, len;
    int opcode;
};
class AsyncWebSocketClient {
public:
    std::vector<String> messages;
    unsigned id() const {
        return 7;
    }
    void text(const String& message) {
        messages.push_back(message);
    }
};
class AsyncWebSocket {
public:
    using Handler = std::function<void(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType, void*, uint8_t*, size_t)>;
    explicit AsyncWebSocket(const String& path) : path(path) {}
    String path;
    Handler handler;
    std::vector<String> broadcasts;
    void onEvent(Handler callback) {
        handler = callback;
    }
    void cleanupClients() {}
    void textAll(const String& message) {
        broadcasts.push_back(message);
    }
    void connect(AsyncWebSocketClient& client) {
        handler(this, &client, WS_EVT_CONNECT, nullptr, nullptr, 0);
    }
    void receive(AsyncWebSocketClient& client, String message, size_t split = 0) {
        AwsFrameInfo frame{true, 0, message.size(), WS_TEXT};
        auto* data = reinterpret_cast<uint8_t*>(message.data());
        if (split) {
            frame.final = false;
            handler(this, &client, WS_EVT_DATA, &frame, data, split);
            frame.final = true;
            frame.index = split;
            handler(this, &client, WS_EVT_DATA, &frame, data + split, message.size() - split);
        } else {
            handler(this, &client, WS_EVT_DATA, &frame, data, message.size());
        }
    }
};
class AsyncWebServerResponse {
public:
    std::map<String, String> headers;
    void addHeader(const String& key, const String& value) {
        headers[key] = value;
    }
};
class AsyncWebServerRequest {
public:
    int status = 0;
    String contentType;
    const uint8_t* data = nullptr;
    size_t size = 0;
    AsyncWebServerResponse response;
    AsyncWebServerResponse* beginResponse(int code, const String& type, const uint8_t* content, size_t length) {
        status = code;
        contentType = type;
        data = content;
        size = length;
        return &response;
    }
    void send(AsyncWebServerResponse*) {}
};
class AsyncWebServer {
public:
    explicit AsyncWebServer(int) {}
    ~AsyncWebServer() {
        delete ws;
    }
    AsyncWebSocket* ws = nullptr;
    std::function<void(AsyncWebServerRequest*)> route;
    void addHandler(AsyncWebSocket* socket) {
        ws = socket;
    }
    template <class T> void on(const String&, int, T callback) {
        route = callback;
    }
};
