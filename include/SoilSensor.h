#pragma once
#include <Arduino.h>

class SoilSensor {
public:
    void begin(uint8_t analogPin, uint8_t samples = 5);
    uint16_t readRaw();                 // singola lettura
    uint16_t readAveraged();            // media N campioni
    void setSamples(uint8_t s) { _samples = s; }
private:
    uint8_t _pin = A0;
    uint8_t _samples = 5;
};
