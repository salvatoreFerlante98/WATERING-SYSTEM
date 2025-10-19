#pragma once
#include <Arduino.h>

class Relay;
class SoilSensor;
class Config;

class Api {
public:
    void begin(uint16_t port, Relay* relay, SoilSensor* soil, const String& deviceId, Config* cfg);
    void update();
private:
    void sendJSON(int code, const String& payload);
    void sendError(int code, const String& msg);
    String buildStateJSON();
};
