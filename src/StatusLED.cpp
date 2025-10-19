#include "StatusLED.h"

void StatusLED::begin(uint8_t pin, bool activeLow) {
    _pin = pin; _activeLow = activeLow;
    pinMode(_pin, OUTPUT);
    set(false);
}

void StatusLED::set(bool on) {
    _steady = true; _state = on;
    digitalWrite(_pin, (_activeLow ? !on : on) ? HIGH : LOW);
}

void StatusLED::blink(uint16_t on_ms, uint16_t off_ms) {
    _steady = false; _on = on_ms; _off = off_ms; _t = millis(); _state = false;
}

void StatusLED::update() {
    if (_steady) return;
    uint32_t now = millis();
    if (_state && (now - _t >= _on)) {
        _state = false; _t = now;
        digitalWrite(_pin, _activeLow ? HIGH : LOW);
    } else if (!_state && (now - _t >= _off)) {
        _state = true; _t = now;
        digitalWrite(_pin, _activeLow ? LOW : HIGH);
    }
}
