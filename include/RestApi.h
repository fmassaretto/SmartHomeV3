#ifndef REST_API_H
#define REST_API_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include "UserManager.h"
#include "DeviceManager.h"
#include "SessionManager.h"

class RestApi {
private:
    AsyncWebServer* server;
    UserManager* userManager;
    DeviceManager* deviceManager;
    SessionManager* sessionManager;
    
    // Setup API routes
    void setupRoutes();
    
    // API handlers
    void handleLogin(AsyncWebServerRequest *request, JsonVariant &json);
    void handleLogout(AsyncWebServerRequest *request);
    void handleGetDevices(AsyncWebServerRequest *request);
    void handleToggleDevice(AsyncWebServerRequest *request, JsonVariant &json);
    void handleGetUsers(AsyncWebServerRequest *request);
    void handleAddUser(AsyncWebServerRequest *request, JsonVariant &json);
    void handleUpdateUser(AsyncWebServerRequest *request, JsonVariant &json);
    void handleDeleteUser(AsyncWebServerRequest *request, JsonVariant &json);
    void handleGetStatus(AsyncWebServerRequest *request);
    
public:
    RestApi(AsyncWebServer* server, UserManager* userManager, DeviceManager* deviceManager, SessionManager* sessionManager);
    
    // Initialize the REST API
    void begin();
};

#endif // REST_API_H
