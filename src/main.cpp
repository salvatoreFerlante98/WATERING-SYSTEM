#include <Arduino.h>
#include "PinMap.h"
#include "Relay.h"
#include "SoilSensor.h"
#include "StatusLED.h"
#include "Console.h"
#include "Net.h"

Relay      relay;
SoilSensor soil;
StatusLED  led;
Console    console;
Net        net;

void setup() {
  relay.begin(PIN_RELAY, /*activeHigh=*/true);
  soil.begin(PIN_SOIL_A0, /*samples=*/5);
  led.begin(PIN_LED, /*activeLow=*/true);
  led.blink(80, 920);

  // === Wi-Fi ===
  net.begin("esp12e-01", /*portalTimeoutSec=*/180);

  console.begin(115200, &relay, &soil, &net);
  Serial.println("Boot OK. Digita HELP.");
}

void loop() {
  relay.update();
  led.update();
  net.update();      // mantiene la connessione
  console.update();
}
