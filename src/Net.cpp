#include "Net.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h>     // gestisce captive portal

void Net::begin(const String& deviceId, uint16_t portalTimeoutSec) {
    _deviceId = deviceId;
    _portalTimeout = portalTimeoutSec;

    WiFi.mode(WIFI_STA);
    // hostname (visibile sul router)
    WiFi.hostname("tri-" + _deviceId);

    WiFiManager wm;
    wm.setConfigPortalTimeout(_portalTimeout);      // es. 3 minuti
    bool ok = wm.autoConnect(("tri-setup-" + _deviceId).c_str());
    if (!ok) {
        // se non configuri entro il timeout, riprova più tardi
        // (restiamo in STA e continuiamo il loop)
    }
}

void Net::update() {
    if (connected()) return;
    // prova riconnessione al massimo ogni 5 secondi
    if (millis() - _lastReconnectTry > 5000) {
        _lastReconnectTry = millis();
        tryReconnect();
    }
}

void Net::tryReconnect() {
    // se abbiamo credenziali salvate, basta chiamare begin() “vuoto”
    // l’SDK usa le ultime credenziali
    WiFi.begin();
}

bool Net::connected() const {
    return WiFi.status() == WL_CONNECTED;
}

String Net::ip() const {
    return connected() ? WiFi.localIP().toString() : String("0.0.0.0");
}

int Net::rssi() const {
    return connected() ? WiFi.RSSI() : 0;
}

void Net::startConfigPortal() {
    WiFiManager wm;
    wm.setConfigPortalTimeout(_portalTimeout);
    // apre un AP tipo "tri-setup-<deviceId>"
    wm.startConfigPortal(("tri-setup-" + _deviceId).c_str());
    // al termine (dopo salvataggio o timeout) ritorna al loop
}

void Net::resetAndReboot() {
    WiFiManager wm;
    wm.resetSettings();     // cancella SSID/PSK salvate
    delay(300);
    ESP.restart();
}
