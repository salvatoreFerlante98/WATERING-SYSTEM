#include <Arduino.h>
#include "PinMap.h"
#include "Relay.h"
#include "SoilSensor.h"
#include "StatusLED.h"
#include "Console.h"
#include "Net.h"
#include "Config.h"
#include "Api.h"

Relay      relay;
SoilSensor soil;
StatusLED  led;
Console    console;
Net        net;
Config     cfg;
Api        api;

void setup() {
  relay.begin(PIN_RELAY, /*activeHigh=*/false);
  soil.begin(PIN_SOIL_A0, /*samples=*/5);
  led.begin(PIN_LED, /*activeLow=*/true);
  led.blink(80, 920);

  // Persistenza & calibrazione
  cfg.begin(&soil, "/config.json");   // LittleFS.begin() incluso qui

  // Wi-Fi
  net.begin("esp12e-01", 180);

  // API
  api.begin(80, &relay, &soil, "esp12e-01", &cfg);

  // Console
  console.begin(115200, &relay, &soil, &net, &cfg);

  Serial.println("Boot OK. Calibrazione & API pronte.");
}

void loop() {
  relay.update();
  led.update();
  net.update();
  api.update();
  console.update();
}
