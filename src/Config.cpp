#include "Config.h"
#include "SoilSensor.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

void Config::begin(SoilSensor* soil, const String& path) {
    _soil = soil; _path = path;
    LittleFS.begin(); // safe: idempotente
    if (!load()) { // prima volta: applica default
        if (_soil) _soil->setCalibration(_dry, _wet);
        save();
    }
}

bool Config::load() {
    if (!LittleFS.exists(_path)) return false;
    File f = LittleFS.open(_path, "r");
    if (!f) return false;

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();
    if (err) return false;

    _dry = doc["soil_dry"] | _dry;
    _wet = doc["soil_wet"] | _wet;
    _threshold = doc["threshold_pct"] | _threshold;

    if (_soil) _soil->setCalibration(_dry, _wet);
    return true;
}

bool Config::save() const {
    File f = LittleFS.open(_path, "w");
    if (!f) return false;

    JsonDocument doc;
    doc["soil_dry"] = _dry;
    doc["soil_wet"] = _wet;
    doc["threshold_pct"] = _threshold;

    bool ok = (serializeJson(doc, f) > 0);
    f.close();
    return ok;
}

void Config::setCalib(uint16_t dry, uint16_t wet) {
    _dry = dry; _wet = wet;
    if (_soil) _soil->setCalibration(_dry, _wet);
}
void Config::getCalib(uint16_t& dry, uint16_t& wet) const { dry=_dry; wet=_wet; }

void Config::setThreshold(float pct) { _threshold = pct; }
float Config::threshold() const { return _threshold; }
