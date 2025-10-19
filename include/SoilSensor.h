#pragma once
#include <Arduino.h>

class SoilSensor {
public:
    void begin(uint8_t analogPin, uint8_t samples = 5);
    uint16_t readRaw();
    uint16_t readAveraged();
    void setSamples(uint8_t s) { _samples = s; }

    // calibrazione & mapping
    void setCalibration(uint16_t dry, uint16_t wet);
    uint16_t dry() const { return _dry; }
    uint16_t wet() const { return _wet; }
    float rawToPercent(uint16_t raw) const;    // 0..100
    float readPercent();                       // media campioni, in %

private:
    uint8_t _pin = A0;
    uint8_t _samples = 5;
    uint16_t _dry = 850;   // da regolare
    uint16_t _wet = 350;   // da regolare
};
