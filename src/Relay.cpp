#include "Relay.h"

void Relay::begin(uint8_t pin, bool activeHigh) {
    _pin = pin;
    _activeHigh = activeHigh;
    pinMode(_pin, OUTPUT);
    off();
}

void Relay::on(uint32_t timeout_ms) {
    digitalWrite(_pin, _activeHigh ? HIGH : LOW);
    _isOn = true;
    _deadline = (timeout_ms > 0) ? (millis() + timeout_ms) : 0;
}

void Relay::off() {
    digitalWrite(_pin, _activeHigh ? LOW : HIGH);
    _isOn = false;
    _deadline = 0;
}

void Relay::update() {
    if (_isOn && _deadline && (millis() > _deadline)) off();
}
