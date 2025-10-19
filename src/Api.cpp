#include "Api.h"
#include "Relay.h"
#include "SoilSensor.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

static ESP8266WebServer* server = nullptr;
static Relay*      s_relay = nullptr;
static SoilSensor* s_soil  = nullptr;
static String      s_deviceId = "esp8266";

// --- Helper per CORS ---
static void setCORS() {
  server->sendHeader("Access-Control-Allow-Origin", "*");
  server->sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

// --- Metodi interni ---
void Api::sendJSON(int code, const String& payload) {
  setCORS();
  server->send(code, "application/json; charset=utf-8", payload);
}

void Api::sendError(int code, const String& msg) {
  JsonDocument doc;
  doc["error"] = msg;
  String out; serializeJson(doc, out);
  sendJSON(code, out);
}

String Api::buildStateJSON() {
  uint16_t raw = s_soil ? s_soil->readAveraged() : 0;

  JsonDocument doc;
  doc["device"]    = s_deviceId;
  doc["ip"]        = WiFi.localIP().toString();
  doc["rssi"]      = WiFi.RSSI();
  doc["uptime_s"]  = (uint32_t)(millis()/1000UL);
  doc["soil_raw"]  = raw;
  doc["pump"]      = s_relay ? s_relay->isOn() : false;

  String out;
  serializeJson(doc, out);
  return out;
}

void Api::begin(uint16_t port, Relay* relay, SoilSensor* soil, const String& deviceId) {
  s_relay = relay;
  s_soil = soil;
  s_deviceId = deviceId;

  server = new ESP8266WebServer(port);

  // --- Routes ---
  server->on("/", HTTP_GET, [] {
    setCORS();
    server->send(200, "text/plain; charset=utf-8",
      "ESP8266 Plant API. Try /api/state, /api/soil, POST /api/pump");
  });

  // CORS preflight + not found
  server->onNotFound([] {
    if (server->method() == HTTP_OPTIONS) {
      setCORS();
      server->send(200, "text/plain", "");
    } else {
      setCORS();
      server->send(404, "text/plain", "Not found");
    }
  });

  // GET /api/state
  server->on("/api/state", HTTP_GET, [this] {
    sendJSON(200, buildStateJSON());
  });

  // GET /api/soil
  server->on("/api/soil", HTTP_GET, [] {
    if (!s_soil) { server->send(500, "text/plain", "soil missing"); return; }
    uint16_t raw = s_soil->readAveraged();
    JsonDocument doc;
    doc["soil_raw"] = raw;
    String out; serializeJson(doc, out);
    setCORS();
    server->send(200, "application/json", out);
  });

  // POST /api/pump  { "on": true, "timeout_ms": 5000 }
  server->on("/api/pump", HTTP_POST, [] {
    if (!s_relay) { server->send(500, "text/plain", "relay missing"); return; }
    String body = server->arg("plain");
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    if (err) { setCORS(); server->send(400, "text/plain", "bad json"); return; }

    bool on = doc["on"] | false;
    uint32_t to = doc["timeout_ms"] | 0;

    if (on) s_relay->on(to);
    else    s_relay->off();

    JsonDocument ack;
    ack["ok"] = true;
    ack["pump"] = on;
    String out; serializeJson(ack, out);
    setCORS();
    server->send(200, "application/json", out);
  });

  server->begin();
}

void Api::update() {
  if (server) server->handleClient();
}
