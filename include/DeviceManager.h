#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <string>
#include <ArduinoJson.h>
#include <LittleFS.h>

// Device structure
struct Device {
    int channel;
    std::vector<int> inputPins;
    std::vector<int> outputPins;
    std::vector<bool> inputState;
    std::vector<bool> outputState;
    std::string name;
    std::string alexaName;  // Name for Alexa integration
    bool alexaEnabled;      // Whether this device is exposed to Alexa
};

class DeviceManager {
private:
    std::vector<Device> devices;
    String configFile = "/devices.json";
    bool initialized = false;
    
    // Save devices to file
    bool saveDevices();
    
    // Load devices from file
    bool loadDevices();
    
public:
    DeviceManager();
    
    // Initialize the device manager
    bool begin();
    
    // Setup device pins
    void setupPins();
    
    // Add a new device
    bool addDevice(const Device& device);
    
    // Update an existing device
    bool updateDevice(int channel, const Device& device);
    
    // Delete a device
    bool deleteDevice(int channel);
    
    // Get all devices
    std::vector<Device>& getAllDevices();
    
    // Get device by channel
    Device* getDeviceByChannel(int channel);
    
    // Toggle device state
    bool toggleDevice(int channel, bool newState = -1);
    
    // Get device state
    bool getDeviceState(int channel);
    
    // Create default devices if none exist
    void createDefaultDevicesIfNeeded();
    
    // Check device inputs (buttons)
    void checkInputs();
};

#endif // DEVICE_MANAGER_H
