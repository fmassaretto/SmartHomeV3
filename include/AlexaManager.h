#ifndef ALEXA_MANAGER_H
#define ALEXA_MANAGER_H

#include <Arduino.h>
#include <Espalexa.h>
#include "DeviceManager.h"

class AlexaManager {
private:
    Espalexa* alexa;
    DeviceManager* deviceManager;
    bool initialized;
    
    // Map to store device IDs by channel
    std::map<int, uint8_t> deviceIds;
    
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
    
    // Device callback (called when Alexa changes device state)
    void deviceCallback(int channel, uint8_t brightness);
};

#endif // ALEXA_MANAGER_H
