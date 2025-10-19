#include "SoilSensor.h"

void SoilSensor::begin(uint8_t analogPin, uint8_t samples) {
    _pin = analogPin;
    _samples = samples;
}

uint16_t SoilSensor::readRaw() {
    return analogRead(_pin);
}

uint16_t SoilSensor::readAveraged() {
    uint32_t acc = 0;
    for (uint8_t i=0;i<_samples;i++) {
        acc += analogRead(_pin); delay(5);
    }
    return acc / _samples;
}
