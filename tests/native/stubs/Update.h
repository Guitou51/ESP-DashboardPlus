#pragma once
#include <Arduino.h>
#include <vector>

struct UpdateStub {
    bool beginOk = true;
    bool endOk = true;
    bool shortWrite = false;
    bool active = false;
    int begins = 0, ends = 0, aborts = 0;
    size_t expectedSize = 0;
    std::vector<uint8_t> bytes;
    bool begin(size_t size) {
        ++begins;
        expectedSize = size;
        bytes.clear();
        active = beginOk;
        return beginOk;
    }
    size_t write(uint8_t* data, size_t size) {
        bytes.insert(bytes.end(), data, data + size);
        return shortWrite ? size - 1 : size;
    }
    bool end(bool) {
        ++ends;
        active = false;
        return endOk;
    }
    void abort() {
        ++aborts;
        active = false;
    }
    template <class T> void printError(T&) {}
};
inline UpdateStub Update;
