#pragma once
#include <Arduino.h>

class Relay;
class SoilSensor;

class Api {
public:
    void begin(uint16_t port, Relay* relay, SoilSensor* soil, const String& deviceId);
    void update(); // chiama handleClient()

private:
    void sendJSON(int code, const String& payload);
    void sendError(int code, const String& msg);
    String buildStateJSON();
};
