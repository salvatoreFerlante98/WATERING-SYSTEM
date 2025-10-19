#pragma once
#include <Arduino.h>

class SoilSensor;

class Config {
public:
    void begin(SoilSensor* soil, const String& path="/config.json");
    bool load();
    bool save() const;

    void setCalib(uint16_t dry, uint16_t wet);
    void getCalib(uint16_t& dry, uint16_t& wet) const;

    void setThreshold(float pct);
    float threshold() const;

private:
    SoilSensor* _soil = nullptr;
    String _path = "/config.json";
    uint16_t _dry = 850;
    uint16_t _wet = 350;
    float _threshold = 35.0f;
};
