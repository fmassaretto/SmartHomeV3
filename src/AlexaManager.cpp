#include "../include/AlexaManager.h"

// Static callback function
void AlexaManager::deviceCallback(uint8_t deviceId, const char* deviceName, bool state) {
    // This is a static method, so we can't access instance variables directly
    // The actual implementation will be handled in the main code
}

// Constructor
AlexaManager::AlexaManager(DeviceManager* deviceManager) {
    this->deviceManager = deviceManager;
    this->alexa = new ESPAlexa();
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
    
    // Start Alexa service
    alexa->begin();
    
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

// Add or update device in Alexa
bool AlexaManager::addOrUpdateDevice(int channel) {
    // Get device from DeviceManager
    Device* device = deviceManager->getDeviceByChannel(channel);
    if (!device || !device->alexaEnabled) {
        return false;
    }
    
    // Check if device already exists in Alexa
    bool deviceExists = false;
    for (uint8_t i = 0; i < alexa->getDeviceCount(); i++) {
        if (strcmp(alexa->getDeviceName(i), device->alexaName.c_str()) == 0) {
            deviceExists = true;
            break;
        }
    }
    
    // Add or update device
    if (deviceExists) {
        // Update device state
        alexa->setState(device->alexaName.c_str(), device->outputState[0]);
    } else {
        // Add new device
        alexa->addDevice(device->alexaName.c_str(), [this, channel](uint8_t deviceId, const char* deviceName, bool state) {
            // Toggle device when Alexa changes state
            deviceManager->toggleDevice(channel, state);
        });
        
        // Set initial state
        alexa->setState(device->alexaName.c_str(), device->outputState[0]);
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
    
    // Remove device from Alexa
    alexa->deleteDevice(device->alexaName.c_str());
    
    return true;
}

// Update all devices in Alexa
void AlexaManager::updateAllDevices() {
    // Clear all devices
    alexa->deleteDevices();
    
    // Add all enabled devices
    for (Device& device : deviceManager->getAllDevices()) {
        if (device.alexaEnabled) {
            alexa->addDevice(device.alexaName.c_str(), [this, channel = device.channel](uint8_t deviceId, const char* deviceName, bool state) {
                // Toggle device when Alexa changes state
                deviceManager->toggleDevice(channel, state);
            });
            
            // Set initial state
            alexa->setState(device.alexaName.c_str(), device.outputState[0]);
        }
    }
}
