#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

class OtaManager {
private:
    AsyncWebServer* server;
    bool otaEnabled;
    
public:
    OtaManager(AsyncWebServer* server);
    
    // Initialize OTA manager
    void begin();
    
    // Enable/disable OTA updates
    void setEnabled(bool enabled);
    
    // Check if OTA is enabled
    bool isEnabled();
    
    // Handle OTA requests (must be called in loop)
    void handle();
};

#endif // OTA_MANAGER_H
