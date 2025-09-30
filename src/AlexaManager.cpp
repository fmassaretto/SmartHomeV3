#include "../include/AlexaManager.h"

// Constructor
AlexaManager::AlexaManager(DeviceManager* deviceManager) {
    this->deviceManager = deviceManager;
    this->alexa = new Espalexa();
    this->initialized = false;
}

// Destructor
AlexaManager::~AlexaManager() {
    delete alexa;
}

// Initialize Alexa integration
bool AlexaManager::begin() {
    if (initialized) {
        return true;
    }
    
    // Add all enabled devices to Alexa
    updateAllDevices();
    
    initialized = true;
    Serial.println("Alexa integration initialized");
    return true;
}

// Handle Alexa events (should be called in loop)
void AlexaManager::handle() {
    if (initialized) {
        alexa->loop();
    }
}

// Device callback (called when Alexa changes device state)
void AlexaManager::deviceCallback(int channel, uint8_t brightness) {
    // Convert brightness to boolean state (on/off)
    bool state = brightness > 0;
    
    // Toggle device when Alexa changes state
    deviceManager->toggleDevice(channel, state);
}

// Add or update device in Alexa
bool AlexaManager::addOrUpdateDevice(int channel) {
    // Get device from DeviceManager
    Device* device = deviceManager->getDeviceByChannel(channel);
    if (!device || !device->alexaEnabled) {
        return false;
    }
    
    // Check if device already exists in Alexa by checking our map
    bool deviceExists = deviceIds.find(channel) != deviceIds.end();
    
    // Add or update device
    if (deviceExists) {
        // Update device state (0 = off, 255 = on)
        alexa->setDeviceState(deviceIds[channel], device->outputState[0] ? 255 : 0);
    } else {
        // Add new device with callback
        uint8_t deviceId = alexa->addDevice(device->alexaName.c_str(), 
            [this, channel](uint8_t brightness) {
                // Call our member function with the channel and brightness
                this->deviceCallback(channel, brightness);
            }, 
            EspalexaDeviceType::onoff);
        
        // Store device ID in our map
        deviceIds[channel] = deviceId;
        
        // Set initial state
        alexa->setDeviceState(deviceId, device->outputState[0] ? 255 : 0);
    }
    
    return true;
}

// Remove device from Alexa
bool AlexaManager::removeDevice(int channel) {
    // Get device from DeviceManager
    Device* device = deviceManager->getDeviceByChannel(channel);
    if (!device) {
        return false;
    }
    
    // Check if device exists in our map
    if (deviceIds.find(channel) != deviceIds.end()) {
        // Remove device from Espalexa
        // Note: Espalexa doesn't have a direct method to remove a single device
        // We'll need to rebuild all devices
        deviceIds.erase(channel);
        updateAllDevices();
    }
    
    return true;
}

// Update all devices in Alexa
void AlexaManager::updateAllDevices() {
    // Clear device IDs map
    deviceIds.clear();
    
    // Clear all devices from Espalexa
    if (initialized) {
        delete alexa;
        alexa = new Espalexa();
    }
    
    // Add all enabled devices
    for (Device& device : deviceManager->getAllDevices()) {
        if (device.alexaEnabled) {
            int channel = device.channel;
            
            // Add device with callback
            uint8_t deviceId = alexa->addDevice(device.alexaName.c_str(), 
                [this, channel](uint8_t brightness) {
                    // Call our member function with the channel and brightness
                    this->deviceCallback(channel, brightness);
                }, 
                EspalexaDeviceType::onoff);
            
            // Store device ID in our map
            deviceIds[channel] = deviceId;
            
            // Set initial state
            alexa->setDeviceState(deviceId, device.outputState[0] ? 255 : 0);
        }
    }
    
    // Begin Espalexa if we're already initialized
    if (initialized) {
        alexa->begin();
    }
}
