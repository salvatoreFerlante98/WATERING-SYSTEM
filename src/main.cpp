#include <Arduino.h>
#include "PinMap.h"
#include "Relay.h"
#include "SoilSensor.h"
#include "StatusLED.h"
#include "Console.h"
#include "Net.h"
#include "Api.h"

Relay      relay;
SoilSensor soil;
StatusLED  led;
Console    console;
Net        net;
Api        api;

void setup() {
  relay.begin(PIN_RELAY, /*activeHigh=*/false);
  soil.begin(PIN_SOIL_A0, /*samples=*/5);
  led.begin(PIN_LED, /*activeLow=*/true);
  led.blink(80, 920);

  net.begin("esp12e-01", 180);
  console.begin(115200, &relay, &soil, &net);

  // Avvia server HTTP API sulla porta 80
  api.begin(80, &relay, &soil, "esp12e-01");

  Serial.println("Boot OK. API online.");
}

void loop() {
  relay.update();
  led.update();
  net.update();
  console.update();
  api.update();
}
