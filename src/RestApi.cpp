#include "../include/RestApi.h"

// Constructor
RestApi::RestApi(AsyncWebServer* server, UserManager* userManager, DeviceManager* deviceManager, SessionManager* sessionManager) {
    this->server = server;
    this->userManager = userManager;
    this->deviceManager = deviceManager;
    this->sessionManager = sessionManager;
}

// Initialize the REST API
void RestApi::begin() {
    setupRoutes();
}

// Setup API routes
void RestApi::setupRoutes() {
    // Login endpoint
    AsyncCallbackJsonWebHandler* loginHandler = new AsyncCallbackJsonWebHandler("/api/login", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        this->handleLogin(request, json);
    });
    server->addHandler(loginHandler);
    
    // Logout endpoint
    server->on("/api/logout", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handleLogout(request);
    });
    
    // Get devices endpoint
    server->on("/api/devices", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetDevices(request);
    });
    
    // Toggle device endpoint
    AsyncCallbackJsonWebHandler* toggleHandler = new AsyncCallbackJsonWebHandler("/api/devices/toggle", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        this->handleToggleDevice(request, json);
    });
    server->addHandler(toggleHandler);
    
    // Get users endpoint (admin only)
    server->on("/api/users", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetUsers(request);
    });
    
    // Add user endpoint (admin only)
    AsyncCallbackJsonWebHandler* addUserHandler = new AsyncCallbackJsonWebHandler("/api/users/add", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        this->handleAddUser(request, json);
    });
    server->addHandler(addUserHandler);
    
    // Update user endpoint (admin only)
    AsyncCallbackJsonWebHandler* updateUserHandler = new AsyncCallbackJsonWebHandler("/api/users/update", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        this->handleUpdateUser(request, json);
    });
    server->addHandler(updateUserHandler);
    
    // Delete user endpoint (admin only)
    AsyncCallbackJsonWebHandler* deleteUserHandler = new AsyncCallbackJsonWebHandler("/api/users/delete", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        this->handleDeleteUser(request, json);
    });
    server->addHandler(deleteUserHandler);
    
    // Get system status endpoint
    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleGetStatus(request);
    });
}

// API handlers
void RestApi::handleLogin(AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();
    
    // Check if username and password are provided
    if (!jsonObj.containsKey("username") || !jsonObj.containsKey("password")) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Username and password are required\"}");
        return;
    }
    
    String username = jsonObj["username"].as<String>();
    String password = jsonObj["password"].as<String>();
    
    // Authenticate user
    if (userManager->authenticate(username, password)) {
        // Create session
        String sessionId = sessionManager->createSession(username);
        
        // Set session cookie
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"success\":true,\"message\":\"Login successful\"}");
        response->addHeader("Set-Cookie", "session=" + sessionId + "; Path=/; Max-Age=3600; HttpOnly");
        request->send(response);
    } else {
        request->send(401, "application/json", "{\"success\":false,\"message\":\"Invalid username or password\"}");
    }
}

void RestApi::handleLogout(AsyncWebServerRequest *request) {
    // Check if request has session cookie
    if (request->hasHeader("Cookie")) {
        String cookie = request->getHeader("Cookie")->value();
        int sessionStart = cookie.indexOf("session=");
        if (sessionStart != -1) {
            sessionStart += 8;  // Length of "session="
            int sessionEnd = cookie.indexOf(";", sessionStart);
            if (sessionEnd == -1) {
                sessionEnd = cookie.length();
            }
            
            String sessionId = cookie.substring(sessionStart, sessionEnd);
            
            // Delete session
            sessionManager->deleteSession(sessionId);
        }
    }
    
    // Clear session cookie
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"success\":true,\"message\":\"Logout successful\"}");
    response->addHeader("Set-Cookie", "session=; Path=/; Max-Age=0; HttpOnly");
    request->send(response);
}

void RestApi::handleGetDevices(AsyncWebServerRequest *request) {
    // Check authentication
    if (!sessionManager->authMiddleware(request, userManager)) {
        request->send(401, "application/json", "{\"success\":false,\"message\":\"Unauthorized\"}");
        return;
    }
    
    // Get username from session
    String cookie = request->getHeader("Cookie")->value();
    int sessionStart = cookie.indexOf("session=");
    sessionStart += 8;  // Length of "session="
    int sessionEnd = cookie.indexOf(";", sessionStart);
    if (sessionEnd == -1) {
        sessionEnd = cookie.length();
    }
    
    String sessionId = cookie.substring(sessionStart, sessionEnd);
    String username = sessionManager->getUsernameFromSession(sessionId);
    
    // Get user role
    UserRole role = userManager->getUserRole(username);
    
    // Create JSON response
    DynamicJsonDocument doc(4096);
    JsonArray devicesArray = doc.createNestedArray("devices");
    
    // Add devices to response
    for (Device& device : deviceManager->getAllDevices()) {
        // Check if user can control this device
        bool canControl = (role == UserRole::ADMIN) || userManager->canControlDevice(username, device.channel);
        bool state = device.outputState[0];
        
        JsonObject deviceObj = devicesArray.createNestedObject();
        deviceObj["channel"] = device.channel;
        deviceObj["name"] = device.name;
        deviceObj["state"] = state;
        deviceObj["canControl"] = canControl;
        deviceObj["alexaEnabled"] = device.alexaEnabled;
    }
    
    // Send response
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void RestApi::handleToggleDevice(AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();
    
    // Check if channel is provided
    if (!jsonObj.containsKey("channel")) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Channel is required\"}");
        return;
    }
    
    int channel = jsonObj["channel"].as<int>();
    bool state = jsonObj.containsKey("state") ? jsonObj["state"].as<bool>() : -1;
    
    // Check if device exists
    Device* device = deviceManager->getDeviceByChannel(channel);
    if (device == nullptr) {
        request->send(404, "application/json", "{\"success\":false,\"message\":\"Device not found\"}");
        return;
    }
    
    // Check if user can control this device
    if (!sessionManager->deviceControlMiddleware(request, userManager, channel)) {
        request->send(403, "application/json", "{\"success\":false,\"message\":\"Permission denied\"}");
        return;
    }
    
    // Toggle device
    if (deviceManager->toggleDevice(channel, state)) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Device toggled\"}");
    } else {
        request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to toggle device\"}");
    }
}

void RestApi::handleGetUsers(AsyncWebServerRequest *request) {
    // Check if user is admin
    if (!sessionManager->adminMiddleware(request, userManager)) {
        request->send(403, "application/json", "{\"success\":false,\"message\":\"Admin permission required\"}");
        return;
    }
    
    // Create JSON response
    DynamicJsonDocument doc(4096);
    JsonArray usersArray = doc.createNestedArray("users");
    
    // Add users to response
    for (User& user : userManager->getAllUsers()) {
        JsonObject userObj = usersArray.createNestedObject();
        userObj["username"] = user.username;
        userObj["role"] = static_cast<int>(user.role);
        
        // Add allowed devices
        JsonArray devicesArray = userObj.createNestedArray("allowedDevices");
        for (int deviceChannel : user.allowedDevices) {
            devicesArray.add(deviceChannel);
        }
    }
    
    // Send response
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void RestApi::handleAddUser(AsyncWebServerRequest *request, JsonVariant &json) {
    // Check if user is admin
    if (!sessionManager->adminMiddleware(request, userManager)) {
        request->send(403, "application/json", "{\"success\":false,\"message\":\"Admin permission required\"}");
        return;
    }
    
    JsonObject jsonObj = json.as<JsonObject>();
    
    // Check if required fields are provided
    if (!jsonObj.containsKey("username") || !jsonObj.containsKey("password") || !jsonObj.containsKey("role")) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Username, password, and role are required\"}");
        return;
    }
    
    String username = jsonObj["username"].as<String>();
    String password = jsonObj["password"].as<String>();
    UserRole role = static_cast<UserRole>(jsonObj["role"].as<int>());
    
    // Get allowed devices
    std::vector<int> allowedDevices;
    if (jsonObj.containsKey("allowedDevices")) {
        JsonArray devicesArray = jsonObj["allowedDevices"].as<JsonArray>();
        for (JsonVariant device : devicesArray) {
            allowedDevices.push_back(device.as<int>());
        }
    }
    
    // Add user
    if (userManager->addUser(username, password, role, allowedDevices)) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"User added\"}");
    } else {
        request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to add user\"}");
    }
}

void RestApi::handleUpdateUser(AsyncWebServerRequest *request, JsonVariant &json) {
    // Check if user is admin
    if (!sessionManager->adminMiddleware(request, userManager)) {
        request->send(403, "application/json", "{\"success\":false,\"message\":\"Admin permission required\"}");
        return;
    }
    
    JsonObject jsonObj = json.as<JsonObject>();
    
    // Check if username is provided
    if (!jsonObj.containsKey("username")) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Username is required\"}");
        return;
    }
    
    String username = jsonObj["username"].as<String>();
    String password = jsonObj.containsKey("password") ? jsonObj["password"].as<String>() : "";
    UserRole role = jsonObj.containsKey("role") ? static_cast<UserRole>(jsonObj["role"].as<int>()) : userManager->getUserRole(username);
    
    // Get allowed devices
    std::vector<int> allowedDevices;
    if (jsonObj.containsKey("allowedDevices")) {
        JsonArray devicesArray = jsonObj["allowedDevices"].as<JsonArray>();
        for (JsonVariant device : devicesArray) {
            allowedDevices.push_back(device.as<int>());
        }
    } else {
        User* user = userManager->getUser(username);
        if (user != nullptr) {
            allowedDevices = user->allowedDevices;
        }
    }
    
    // Update user
    if (userManager->updateUser(username, password, role, allowedDevices)) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"User updated\"}");
    } else {
        request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to update user\"}");
    }
}

void RestApi::handleDeleteUser(AsyncWebServerRequest *request, JsonVariant &json) {
    // Check if user is admin
    if (!sessionManager->adminMiddleware(request, userManager)) {
        request->send(403, "application/json", "{\"success\":false,\"message\":\"Admin permission required\"}");
        return;
    }
    
    JsonObject jsonObj = json.as<JsonObject>();
    
    // Check if username is provided
    if (!jsonObj.containsKey("username")) {
        request->send(400, "application/json", "{\"success\":false,\"message\":\"Username is required\"}");
        return;
    }
    
    String username = jsonObj["username"].as<String>();
    
    // Delete user
    if (userManager->deleteUser(username)) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"User deleted\"}");
    } else {
        request->send(500, "application/json", "{\"success\":false,\"message\":\"Failed to delete user\"}");
    }
}

void RestApi::handleGetStatus(AsyncWebServerRequest *request) {
    // Check authentication
    if (!sessionManager->authMiddleware(request, userManager)) {
        request->send(401, "application/json", "{\"success\":false,\"message\":\"Unauthorized\"}");
        return;
    }
    
    // Create JSON response
    DynamicJsonDocument doc(1024);
    doc["wifi"]["connected"] = WiFi.status() == WL_CONNECTED;
    doc["wifi"]["ssid"] = WiFi.SSID();
    doc["wifi"]["rssi"] = WiFi.RSSI();
    doc["wifi"]["ip"] = WiFi.localIP().toString();
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    
    // Send response
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
