#include "../include/OtaManager.h"

// Constructor
OtaManager::OtaManager(AsyncWebServer* server) {
    this->server = server;
    this->otaEnabled = false;
}

// Initialize OTA manager
void OtaManager::begin() {
    // Setup ElegantOTA
    ElegantOTA.begin(server); // Initialize ElegantOTA
    
    // Set authentication
    ElegantOTA.setAuth("admin", "admin123");
    
    // Add security middleware to OTA endpoints
    server->on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Check if OTA is enabled
        if (!this->otaEnabled) {
            request->send(403, "text/plain", "OTA updates are disabled");
            return;
        }
        
        // Check if user is authenticated as admin
        if (!request->authenticate("admin", "admin123")) {
            return request->requestAuthentication();
        }
        
        // Allow access to OTA update page
        request->send(200, "text/html", "<html><head><title>ESP32 OTA Update</title></head><body><h1>ESP32 OTA Update</h1><div id='ota-update'></div><script src='/ElegantOTA.js'></script></body></html>");
    });
    
    Serial.println("OTA manager initialized");
}

// Enable/disable OTA updates
void OtaManager::setEnabled(bool enabled) {
    this->otaEnabled = enabled;
    Serial.println(enabled ? "OTA updates enabled" : "OTA updates disabled");
}

// Check if OTA is enabled
bool OtaManager::isEnabled() {
    return this->otaEnabled;
}

// Handle OTA requests (must be called in loop)
void OtaManager::handle() {
    ElegantOTA.loop();
}
