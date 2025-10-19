#pragma once
#include <Arduino.h>

class StatusLED {
public:
    void begin(uint8_t pin, bool activeLow = true);
    void set(bool on);
    void blink(uint16_t on_ms, uint16_t off_ms); // pattern
    void update();
private:
    uint8_t _pin=LED_BUILTIN; bool _activeLow=true;
    bool _steady=false; bool _state=false;
    uint16_t _on=100, _off=900; uint32_t _t=0;
};
