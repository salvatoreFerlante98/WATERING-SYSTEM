#include "Console.h"

void Console::begin(uint32_t baud, Relay* r, SoilSensor* s, Net* n, Config* c) {
  Serial.begin(baud);
  delay(200);
  _relay = r;
  _soil  = s;
  _net   = n;
  _cfg   = c;
  help();
}

void Console::help() {
  Serial.println(F("\n== Console =="));
  Serial.println(F("HELP                      - mostra comandi"));
  Serial.println(F("READ                      - leggi suolo (raw, %)"));
  Serial.println(F("PUMP ON [ms] / PUMP OFF   - pompa"));
  Serial.println(F("WIFI STATUS/CONFIG/RESET  - gestione Wi-Fi"));
  Serial.println(F("CALIB SHOW                - mostra dry/wet/threshold"));
  Serial.println(F("CALIB DRY <raw>           - imposta dry"));
  Serial.println(F("CALIB WET <raw>           - imposta wet"));
  Serial.println(F("THRESH <pct>              - soglia percentuale"));
  Serial.println(F("SAVE / LOAD               - persistenza calibrazione"));
}

void Console::update() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      String line = _buf;
      _buf = "";
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
    if (_soil) {
      uint16_t raw = _soil->readAveraged();
      float pct = _soil->rawToPercent(raw);
      Serial.printf("Soil raw: %u  |  Soil %%: %.1f\n", raw, pct);
    } else {
      Serial.println("Soil sensor not available.");
    }
    return;
  }

  if (line.startsWith("PUMP ")) {
    if (!_relay) { Serial.println("Relay not available."); return; }
    if (line.endsWith("OFF")) {
      _relay->off();
      Serial.println("Pump: OFF");
      return;
    }
    if (line.startsWith("PUMP ON")) {
      int ms = 0;
      int sp = line.indexOf(' ', 7);
      if (sp > 0) ms = line.substring(sp+1).toInt();
      if (ms <= 0) ms = 5000;
      _relay->on((uint32_t)ms);
      Serial.printf("Pump: ON (%d ms)\n", ms);
      return;
    }
  }

  if (line.equalsIgnoreCase("WIFI STATUS")) {
    if (_net) {
      Serial.printf("WiFi: %s  IP: %s  RSSI: %d dBm\n",
        _net->connected() ? "CONNECTED" : "DISCONNECTED",
        _net->ip().c_str(),
        _net->rssi());
    } else {
      Serial.println("Net not available.");
    }
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

  if (line.equalsIgnoreCase("CALIB SHOW")) {
    if (_cfg) {
      uint16_t d,w; _cfg->getCalib(d,w);
      Serial.printf("Calib: DRY=%u  WET=%u  THRESH=%.1f%%\n", d, w, _cfg->threshold());
    } else Serial.println("Config not available.");
    return;
  }

  if (line.startsWith("CALIB DRY ")) {
    if (_cfg) {
      uint16_t v = (uint16_t)line.substring(10).toInt();
      uint16_t d,w; _cfg->getCalib(d,w);
      _cfg->setCalib(v, w);
      Serial.printf("Set DRY=%u\n", v);
    } else Serial.println("Config not available.");
    return;
  }

  if (line.startsWith("CALIB WET ")) {
    if (_cfg) {
      uint16_t v = (uint16_t)line.substring(10).toInt();
      uint16_t d,w; _cfg->getCalib(d,w);
      _cfg->setCalib(d, v);
      Serial.printf("Set WET=%u\n", v);
    } else Serial.println("Config not available.");
    return;
  }

  if (line.startsWith("THRESH ")) {
    if (_cfg) {
      float p = line.substring(7).toFloat();
      p = constrain(p, 0.0f, 100.0f);  // funzione built-in Arduino
      _cfg->setThreshold(p);
      Serial.printf("Set THRESH=%.1f%%\n", p);
    } else Serial.println("Config not available.");
    return;
  }

  if (line.equalsIgnoreCase("SAVE")) {
    if (_cfg) { bool ok = _cfg->save(); Serial.println(ok ? "Saved." : "Save failed!"); }
    else Serial.println("Config not available.");
    return;
  }

  if (line.equalsIgnoreCase("LOAD")) {
    if (_cfg) { bool ok = _cfg->load(); Serial.println(ok ? "Loaded." : "Load failed!"); }
    else Serial.println("Config not available.");
    return;
  }

  Serial.println("Comando non valido. Digita HELP.");
}
