#include "Api.h"
#include "Relay.h"
#include "SoilSensor.h"
#include "Config.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

static ESP8266WebServer* server = nullptr;
static Relay*      s_relay = nullptr;
static SoilSensor* s_soil  = nullptr;
static Config*     s_cfg   = nullptr;
static String      s_deviceId = "esp8266";

static void setCORS() {
  server->sendHeader("Access-Control-Allow-Origin", "*");
  server->sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void Api::sendJSON(int code, const String& payload) {
  setCORS();
  server->send(code, "application/json; charset=utf-8", payload);
}
void Api::sendError(int code, const String& msg) {
  JsonDocument doc; doc["error"] = msg;
  String out; serializeJson(doc, out);
  sendJSON(code, out);
}

String Api::buildStateJSON() {
  uint16_t raw = s_soil ? s_soil->readAveraged() : 0;
  float pct    = s_soil ? s_soil->rawToPercent(raw) : 0.0f;

  JsonDocument doc;
  doc["device"]    = s_deviceId;
  doc["ip"]        = WiFi.localIP().toString();
  doc["rssi"]      = WiFi.RSSI();
  doc["uptime_s"]  = (uint32_t)(millis()/1000UL);
  doc["soil_raw"]  = raw;
  doc["soil_pct"]  = pct;
  doc["pump"]      = s_relay ? s_relay->isOn() : false;
  if (s_cfg) {
    doc["calib"]["dry"] = s_cfg->threshold(); // non mischiamo: aggiungiamo sotto
  }
  // meglio esplicitare calibrazione:
  if (s_soil && s_cfg) {
    uint16_t dry, wet; s_cfg->getCalib(dry, wet);
    doc["calib"]["dry"] = dry;
    doc["calib"]["wet"] = wet;
    doc["threshold_pct"] = s_cfg->threshold();
  }

  String out; serializeJson(doc, out);
  return out;
}

void Api::begin(uint16_t port, Relay* relay, SoilSensor* soil, const String& deviceId, Config* cfg) {
  s_relay = relay; s_soil = soil; s_deviceId = deviceId; s_cfg = cfg;
  server = new ESP8266WebServer(port);

  // Root
  server->on("/", HTTP_GET, []{
    setCORS();
    server->send(200, "text/plain; charset=utf-8",
      "ESP8266 Plant API: /api/state, /api/soil, /api/pump, /api/calib");
  });

  // CORS preflight + 404
  server->onNotFound([]{
    if (server->method() == HTTP_OPTIONS) { setCORS(); server->send(200, "text/plain", ""); }
    else { setCORS(); server->send(404, "text/plain", "Not found"); }
  });

  // Stato
  server->on("/api/state", HTTP_GET, [this]{ sendJSON(200, buildStateJSON()); });

  // Soil raw
  server->on("/api/soil", HTTP_GET, []{
    if (!s_soil) { server->send(500, "text/plain", "soil missing"); return; }
    uint16_t raw = s_soil->readAveraged();
    JsonDocument doc; doc["soil_raw"] = raw; doc["soil_pct"] = s_soil->rawToPercent(raw);
    String out; serializeJson(doc, out);
    setCORS(); server->send(200, "application/json", out);
  });

  // Pompa
  server->on("/api/pump", HTTP_POST, []{
    if (!s_relay) { server->send(500, "text/plain", "relay missing"); return; }
    String body = server->arg("plain");
    JsonDocument doc; DeserializationError err = deserializeJson(doc, body);
    if (err) { setCORS(); server->send(400, "text/plain", "bad json"); return; }

    bool on = doc["on"] | false;
    uint32_t to = doc["timeout_ms"] | 0;
    if (on) s_relay->on(to); else s_relay->off();

    JsonDocument ack; ack["ok"]=true; ack["pump"]=on;
    String out; serializeJson(ack, out);
    setCORS(); server->send(200, "application/json", out);
  });

  // Calibrazione (GET/POST)
  // GET /api/calib -> {dry, wet, threshold_pct}
  server->on("/api/calib", HTTP_GET, []{
    if (!s_cfg) { server->send(500, "text/plain", "config missing"); return; }
    uint16_t dry, wet; s_cfg->getCalib(dry, wet);
    JsonDocument doc; doc["dry"]=dry; doc["wet"]=wet; doc["threshold_pct"]=s_cfg->threshold();
    String out; serializeJson(doc, out);
    setCORS(); server->send(200, "application/json", out);
  });

  // POST /api/calib  { "dry":900, "wet":300, "threshold_pct":35, "save":true }
  server->on("/api/calib", HTTP_POST, []{
    if (!s_cfg || !s_soil) { server->send(500, "text/plain", "config/soil missing"); return; }
    String body = server->arg("plain");
    JsonDocument doc; DeserializationError err = deserializeJson(doc, body);
    if (err) { setCORS(); server->send(400, "text/plain", "bad json"); return; }

    bool changed = false;

    // dry/wet: usa .is<T>() invece di containsKey()
    auto jDry = doc["dry"];
    auto jWet = doc["wet"];
    if (jDry.is<unsigned>() && jWet.is<unsigned>()) {
      uint16_t dry = (uint16_t) jDry.as<unsigned>();
      uint16_t wet = (uint16_t) jWet.as<unsigned>();
      s_cfg->setCalib(dry, wet);
      changed = true;
    }

    // threshold_pct: accetta numerico (int/float/double)
    auto jTh = doc["threshold_pct"];
    if (jTh.is<float>() || jTh.is<double>() || jTh.is<long>() || jTh.is<int>()) {
      float th = jTh.as<float>();
      // piccolo clamp per sicurezza
      if (th < 0) th = 0;
      if (th > 100) th = 100;
      s_cfg->setThreshold(th);
      changed = true;
    }

    bool doSave = doc["save"] | false;
    bool saved = doSave ? s_cfg->save() : false;

    JsonDocument ack;
    ack["ok"] = true;
    ack["changed"] = changed;
    ack["saved"] = saved;
    uint16_t dry, wet; s_cfg->getCalib(dry, wet);
    ack["dry"] = dry;
    ack["wet"] = wet;
    ack["threshold_pct"] = s_cfg->threshold();

    String out; serializeJson(ack, out);
    setCORS(); server->send(200, "application/json", out);
  });

  server->begin();
}

void Api::update() {
  if (server) server->handleClient();
}
