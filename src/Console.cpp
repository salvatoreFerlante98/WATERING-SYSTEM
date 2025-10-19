#include "Console.h"

void Console::begin(uint32_t baud, Relay* r, SoilSensor* s, Net* n) {
  Serial.begin(baud);
  delay(200);
  _relay = r; _soil = s; _net = n;
  help();
}

void Console::help() {
  Serial.println(F("\n== Console =="));
  Serial.println(F("HELP                  - mostra comandi"));
  Serial.println(F("READ                  - leggi suolo (media)"));
  Serial.println(F("PUMP ON [ms]         - accendi pompa (timeout opz.)"));
  Serial.println(F("PUMP OFF             - spegni pompa"));
  Serial.println(F("WIFI STATUS          - stato Wi-Fi (IP, RSSI)"));
  Serial.println(F("WIFI CONFIG          - avvia portal di configurazione"));
  Serial.println(F("WIFI RESET           - cancella credenziali e riavvia"));
}

void Console::update() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      String line = _buf; _buf = "";
      line.trim();
      if (line.length()) handleLine(line);
    } else {
      _buf += c;
    }
  }
}

void Console::handleLine(const String& line) {
  if (line.equalsIgnoreCase("HELP")) { help(); return; }
  if (line.equalsIgnoreCase("READ")) {
    uint16_t raw = _soil->readAveraged();
    Serial.printf("Soil raw: %u (0=wet .. 1023=dry)\n", raw);
    return;
  }
  if (line.startsWith("PUMP ")) {
    if (line.endsWith("OFF")) { _relay->off(); Serial.println("Pump: OFF"); return; }
    if (line.startsWith("PUMP ON")) {
      int ms = 0; int sp = line.indexOf(' ', 7);
      if (sp > 0) ms = line.substring(sp+1).toInt();
      if (ms <= 0) ms = 5000;
      _relay->on((uint32_t)ms);
      Serial.printf("Pump: ON (%d ms)\n", ms);
      return;
    }
  }
  if (line.equalsIgnoreCase("WIFI STATUS")) {
    Serial.printf("WiFi: %s  IP: %s  RSSI: %d dBm\n",
                  (_net && _net->connected()) ? "CONNECTED" : "DISCONNECTED",
                  _net ? _net->ip().c_str() : "n/a",
                  _net ? _net->rssi() : 0);
    return;
  }
  if (line.equalsIgnoreCase("WIFI CONFIG")) {
    Serial.println("Opening config portal...");
    if (_net) _net->startConfigPortal();
    Serial.println("Portal closed.");
    return;
  }
  if (line.equalsIgnoreCase("WIFI RESET")) {
    Serial.println("Resetting Wi-Fi settings and rebooting...");
    if (_net) _net->resetAndReboot();
    return;
  }
  Serial.println("Comando non valido. Digita HELP.");
}
