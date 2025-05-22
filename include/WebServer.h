#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "WiFiManager.h"
#include "UserManager.h"
#include "DeviceManager.h"
#include "SessionManager.h"
#include "RestApi.h"

class WebServer {
private:
    AsyncWebServer* server;
    WiFiManager* wifiManager;
    UserManager* userManager;
    DeviceManager* deviceManager;
    SessionManager* sessionManager;
    RestApi* restApi;
    AsyncEventSource* events;
    
    // Setup web routes
    void setupRoutes();
    
    // Serve static files from LittleFS
    void serveStatic();
    
public:
    WebServer(WiFiManager* wifiManager, UserManager* userManager, DeviceManager* deviceManager);
    
    // Initialize the web server
    void begin();
    
    // Send device state update event
    void sendDeviceStateEvent(int channel, bool state);
};

#endif // WEB_SERVER_H
