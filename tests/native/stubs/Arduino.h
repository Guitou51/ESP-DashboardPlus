#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

using String = std::string;
inline unsigned long testMillis = 12345678;
inline unsigned long testDelay = 0;
inline unsigned long millis() {
    return testMillis;
}
inline void delay(unsigned long ms) {
    testDelay += ms;
}
template <class T> T constrain(T value, T low, T high) {
    return std::min(std::max(value, low), high);
}
struct SerialStub {
    template <class... Args> void printf(const char*, Args...) {}
    template <class T> void print(const T&) {}
    template <class T> void println(const T&) {}
};
inline SerialStub Serial;
struct EspStub {
    int restarts = 0;
    void restart() {
        ++restarts;
    }
};
inline EspStub ESP;
