#include "../include/OtaManager.h"

// Constructor
OtaManager::OtaManager(AsyncWebServer* server) {
    this->server = server;
    this->otaEnabled = false;
}

// Initialize OTA manager
void OtaManager::begin() {
    // Setup AsyncElegantOTA
    AsyncElegantOTA.begin(server, "admin", "admin123"); // Default credentials
    
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
        ElegantOTA.handleRequest(request);
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
