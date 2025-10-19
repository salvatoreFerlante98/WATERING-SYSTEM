#include "SoilSensor.h"

void SoilSensor::begin(uint8_t analogPin, uint8_t samples) {
    _pin = analogPin; _samples = samples;
}

uint16_t SoilSensor::readRaw() {
    return analogRead(_pin);
}

uint16_t SoilSensor::readAveraged() {
    uint32_t acc = 0;
    for (uint8_t i=0;i<_samples;i++) { acc += analogRead(_pin); delay(5); }
    return acc / _samples;
}

void SoilSensor::setCalibration(uint16_t dry, uint16_t wet) {
    _dry = dry; _wet = wet;
}

static inline float clampF(float v, float lo, float hi){ return v<lo?lo:(v>hi?hi:v); }

float SoilSensor::rawToPercent(uint16_t raw) const {
    // mappa linearmente: dry → 0%, wet → 100%
    int32_t span = (int32_t)_dry - (int32_t)_wet; // >0 se dry>wet
    if (span == 0) return 0.0f;
    float pct = 100.0f * ((int32_t)_dry - (int32_t)raw) / (float)span;
    return clampF(pct, 0.0f, 100.0f);
}

float SoilSensor::readPercent() {
    return rawToPercent(readAveraged());
}
