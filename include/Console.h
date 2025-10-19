#pragma once
#include <Arduino.h>
#include "Relay.h"
#include "SoilSensor.h"
#include "Net.h"
#include "Config.h"

class Console {
public:
    void begin(uint32_t baud, Relay* r, SoilSensor* s, Net* n, Config* c);
    void update();
private:
    String _buf;
    Relay* _relay = nullptr;
    SoilSensor* _soil = nullptr;
    Net* _net = nullptr;
    Config* _cfg = nullptr;

    void handleLine(const String& line);
    void help();
};
