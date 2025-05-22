#include "../include/DeviceManager.h"

// Constructor
DeviceManager::DeviceManager() {
    initialized = false;
}

// Initialize the device manager
bool DeviceManager::begin() {
    // Initialize LittleFS if not already initialized
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    
    // Load devices from file
    if (!loadDevices()) {
        Serial.println("Failed to load devices, creating default configuration");
        // Create default devices
        createDefaultDevicesIfNeeded();
        // Save devices to file
        saveDevices();
    }
    
    // Setup device pins
    setupPins();
    
    initialized = true;
    return true;
}

// Setup device pins
void DeviceManager::setupPins() {
    for (auto& device : devices) {
        // Setup input pins
        for (int pin : device.inputPins) {
            pinMode(pin, INPUT_PULLUP);
        }
        
        // Setup output pins
        for (size_t i = 0; i < device.outputPins.size(); i++) {
            pinMode(device.outputPins[i], OUTPUT);
            digitalWrite(device.outputPins[i], device.outputState[i] ? LOW : HIGH);  // HIGH = OFF, LOW = ON
        }
    }
}

// Save devices to file
bool DeviceManager::saveDevices() {
    // Create a JSON document
    DynamicJsonDocument doc(4096);
    JsonArray devicesArray = doc.createNestedArray("devices");
    
    // Add each device to the JSON document
    for (const Device& device : devices) {
        JsonObject deviceObj = devicesArray.createNestedObject();
        deviceObj["channel"] = device.channel;
        deviceObj["name"] = device.name;
        deviceObj["alexaName"] = device.alexaName;
        deviceObj["alexaEnabled"] = device.alexaEnabled;
        
        // Add input pins
        JsonArray inputPinsArray = deviceObj.createNestedArray("inputPins");
        for (int pin : device.inputPins) {
            inputPinsArray.add(pin);
        }
        
        // Add output pins
        JsonArray outputPinsArray = deviceObj.createNestedArray("outputPins");
        for (int pin : device.outputPins) {
            outputPinsArray.add(pin);
        }
        
        // Add output states
        JsonArray outputStateArray = deviceObj.createNestedArray("outputState");
        for (bool state : device.outputState) {
            outputStateArray.add(state);
        }
    }
    
    // Open the file for writing
    File file = LittleFS.open(configFile, "w");
    if (!file) {
        Serial.println("Failed to open devices file for writing");
        return false;
    }
    
    // Serialize JSON to file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write devices to file");
        file.close();
        return false;
    }
    
    file.close();
    return true;
}

// Load devices from file
bool DeviceManager::loadDevices() {
    // Check if file exists
    if (!LittleFS.exists(configFile)) {
        Serial.println("Devices file does not exist");
        return false;
    }
    
    // Open the file for reading
    File file = LittleFS.open(configFile, "r");
    if (!file) {
        Serial.println("Failed to open devices file for reading");
        return false;
    }
    
    // Parse JSON
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.print("Failed to parse devices file: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Clear existing devices
    devices.clear();
    
    // Read devices from JSON
    JsonArray devicesArray = doc["devices"];
    for (JsonObject deviceObj : devicesArray) {
        Device device;
        device.channel = deviceObj["channel"].as<int>();
        device.name = deviceObj["name"].as<std::string>();
        device.alexaName = deviceObj["alexaName"].as<std::string>();
        device.alexaEnabled = deviceObj["alexaEnabled"].as<bool>();
        
        // Read input pins
        JsonArray inputPinsArray = deviceObj["inputPins"];
        for (int pin : inputPinsArray) {
            device.inputPins.push_back(pin);
        }
        
        // Read output pins
        JsonArray outputPinsArray = deviceObj["outputPins"];
        for (int pin : outputPinsArray) {
            device.outputPins.push_back(pin);
        }
        
        // Read output states
        JsonArray outputStateArray = deviceObj["outputState"];
        for (bool state : outputStateArray) {
            device.outputState.push_back(state);
        }
        
        // Initialize input states
        device.inputState.resize(device.inputPins.size(), false);
        
        devices.push_back(device);
    }
    
    return true;
}

// Add a new device
bool DeviceManager::addDevice(const Device& device) {
    // Check if device already exists
    for (const Device& existingDevice : devices) {
        if (existingDevice.channel == device.channel) {
            return false;
        }
    }
    
    // Add device to list
    devices.push_back(device);
    
    // Save devices to file
    return saveDevices();
}

// Update an existing device
bool DeviceManager::updateDevice(int channel, const Device& device) {
    // Find device
    for (size_t i = 0; i < devices.size(); i++) {
        if (devices[i].channel == channel) {
            // Update device
            devices[i] = device;
            
            // Save devices to file
            return saveDevices();
        }
    }
    
    return false;
}

// Delete a device
bool DeviceManager::deleteDevice(int channel) {
    // Find device
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if (it->channel == channel) {
            // Remove device
            devices.erase(it);
            
            // Save devices to file
            return saveDevices();
        }
    }
    
    return false;
}

// Get all devices
std::vector<Device>& DeviceManager::getAllDevices() {
    return devices;
}

// Get device by channel
Device* DeviceManager::getDeviceByChannel(int channel) {
    for (Device& device : devices) {
        if (device.channel == channel) {
            return &device;
        }
    }
    
    return nullptr;
}

// Toggle device state
bool DeviceManager::toggleDevice(int channel, bool newState) {
    // Find device
    Device* device = getDeviceByChannel(channel);
    if (device == nullptr) {
        return false;
    }
    
    // Toggle state if newState is -1, otherwise set to newState
    if (newState == -1) {
        device->outputState[0] = !device->outputState[0];
    } else {
        device->outputState[0] = newState;
    }
    
    // Set output pin
    digitalWrite(device->outputPins[0], device->outputState[0] ? LOW : HIGH);  // HIGH = OFF, LOW = ON
    
    // Save devices to file
    saveDevices();
    
    return true;
}

// Get device state
bool DeviceManager::getDeviceState(int channel) {
    // Find device
    Device* device = getDeviceByChannel(channel);
    if (device == nullptr) {
        return false;
    }
    
    return device->outputState[0];
}

// Create default devices if none exist
void DeviceManager::createDefaultDevicesIfNeeded() {
    if (devices.empty()) {
        // Create default devices based on the original code
        Device device1;
        device1.channel = 0;
        device1.inputPins = {25};
        device1.outputPins = {21};
        device1.inputState = {false};
        device1.outputState = {false};
        device1.name = "Luz_Cozinha";
        device1.alexaName = "Kitchen Light";
        device1.alexaEnabled = true;
        devices.push_back(device1);
        
        Device device2;
        device2.channel = 1;
        device2.inputPins = {33};
        device2.outputPins = {22};
        device2.inputState = {false};
        device2.outputState = {false};
        device2.name = "Luz_Lavanderia";
        device2.alexaName = "Laundry Light";
        device2.alexaEnabled = true;
        devices.push_back(device2);
        
        Device device3;
        device3.channel = 2;
        device3.inputPins = {32};
        device3.outputPins = {23};
        device3.inputState = {false};
        device3.outputState = {false};
        device3.name = "Luz_Corredor_Quintal";
        device3.alexaName = "Corridor Light";
        device3.alexaEnabled = true;
        devices.push_back(device3);
        
        Device device4;
        device4.channel = 3;
        device4.inputPins = {26, 27};
        device4.outputPins = {19};
        device4.inputState = {false, false};
        device4.outputState = {false};
        device4.name = "Luz_Quarto";
        device4.alexaName = "Bedroom Light";
        device4.alexaEnabled = true;
        devices.push_back(device4);
        
        Serial.println("Created default devices");
    }
}

// Check device inputs (buttons)
void DeviceManager::checkInputs() {
    for (Device& device : devices) {
        for (size_t i = 0; i < device.inputPins.size(); i++) {
            bool currentState = digitalRead(device.inputPins[i]) == HIGH;
            
            // Trigger only on rising edge
            if (currentState && !device.inputState[i]) {
                toggleDevice(device.channel);
            }
            
            // Save input state for edge detection
            device.inputState[i] = currentState;
        }
    }
}
