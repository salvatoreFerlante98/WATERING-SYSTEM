#pragma once
#include <Arduino.h>

class Net {
public:
    void begin(const String& deviceId, uint16_t portalTimeoutSec = 180);
    void update();                       // riconnessione “gentile”
    bool connected() const;
    String ip() const;
    int rssi() const;
    void startConfigPortal();             // avvia AP di configurazione
    void resetAndReboot();                // dimentica SSID salvate e riavvia
private:
    String _deviceId = "esp8266";
    uint32_t _lastReconnectTry = 0;
    uint16_t _portalTimeout = 180;
    void tryReconnect();
};
