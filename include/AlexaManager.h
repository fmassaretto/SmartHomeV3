#ifndef ALEXA_MANAGER_H
#define ALEXA_MANAGER_H

#include <Arduino.h>
#include <ESPAlexa.h>
#include "DeviceManager.h"

class AlexaManager {
private:
    Espalexa* alexa;
    DeviceManager* deviceManager;
    bool initialized;
    
    // Callback for Alexa device state changes
    static void deviceCallback(uint8_t deviceId, const char* deviceName, bool state);
    
public:
    AlexaManager(DeviceManager* deviceManager);
    ~AlexaManager();
    
    // Initialize Alexa integration
    bool begin();
    
    // Handle Alexa events (should be called in loop)
    void handle();
    
    // Add or update device in Alexa
    bool addOrUpdateDevice(int channel);
    
    // Remove device from Alexa
    bool removeDevice(int channel);
    
    // Update all devices in Alexa
    void updateAllDevices();
};

#endif // ALEXA_MANAGER_H
