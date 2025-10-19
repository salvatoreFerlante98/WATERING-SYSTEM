#include <Arduino.h>
#include "PinMap.h"
#include "Relay.h"
#include "SoilSensor.h"
#include "StatusLED.h"
#include "Console.h"

Relay      relay;
SoilSensor soil;
StatusLED  led;
Console    console;

void setup() {
  relay.begin(PIN_RELAY, /*activeHigh=*/false);      // se il tuo relè è active-LOW, metti false
  soil.begin(PIN_SOIL_A0, /*samples=*/5);
  led.begin(PIN_LED, /*activeLow=*/true);
  led.blink(80, 920);                               // heartbeat lento
  console.begin(115200, &relay, &soil);

  Serial.println("Boot OK. Digita HELP.");
}

void loop() {
  relay.update(); // gestisce timeout
  led.update();      // animazione LED
  console.update();  // comandi seriali
}
