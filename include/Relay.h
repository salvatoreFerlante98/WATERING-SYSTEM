#pragma once
#include <Arduino.h>

class Relay {
public:
    void begin(uint8_t pin, bool activeHigh = true);
    void on(uint32_t timeout_ms = 0);
    void off();
    void update();
    bool isOn() const { return _isOn; }
private:
    uint8_t  _pin = 0;
    bool     _activeHigh = true;
    bool     _isOn = false;
    uint32_t _deadline = 0; // 0 = no timeout
};
