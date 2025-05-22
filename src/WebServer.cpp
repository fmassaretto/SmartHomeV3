#include "../include/WebServer.h"
#include <LittleFS.h>

// Constructor
WebServer::WebServer(WiFiManager* wifiManager, UserManager* userManager, DeviceManager* deviceManager) {
    this->wifiManager = wifiManager;
    this->userManager = userManager;
    this->deviceManager = deviceManager;
    
    // Create server instance
    this->server = new AsyncWebServer(80);
    
    // Create session manager
    this->sessionManager = new SessionManager();
    
    // Create REST API
    this->restApi = new RestApi(server, userManager, deviceManager, sessionManager);
    
    // Create event source
    this->events = new AsyncEventSource("/events");
}

// Initialize the web server
void WebServer::begin() {
    // Initialize REST API
    restApi->begin();
    
    // Setup web routes
    setupRoutes();
    
    // Serve static files
    serveStatic();
    
    // Setup OTA updates
    ElegantOTA.begin(&server);
    
    // Add event handler
    events->onConnect([this](AsyncEventSourceClient *client) {
        // Send current device states when client connects
        for (Device& device : deviceManager->getAllDevices()) {
            this->sendDeviceStateEvent(device.channel, device.outputState[0]);
        }
    });
    server->addHandler(events);
    
    // Start server
    server->begin();
    
    Serial.println("Web server started");
}

// Setup web routes
void WebServer::setupRoutes() {
    // Serve index page
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    
    // Serve login page
    server->on("/login", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/login.html", "text/html");
    });
    
    // Serve admin page
    server->on("/admin", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (sessionManager->adminMiddleware(request, userManager)) {
            request->send(LittleFS, "/admin.html", "text/html");
        } else {
            request->redirect("/login");
        }
    });
    
    // Handle 404
    server->onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
}

// Serve static files from LittleFS
void WebServer::serveStatic() {
    // Serve CSS files
    server->serveStatic("/css/", LittleFS, "/css/");
    
    // Serve JavaScript files
    server->serveStatic("/js/", LittleFS, "/js/");
    
    // Serve images
    server->serveStatic("/img/", LittleFS, "/img/");
}

// Send device state update event
void WebServer::sendDeviceStateEvent(int channel, bool state) {
    // Create JSON message
    DynamicJsonDocument doc(128);
    doc["channel"] = channel;
    doc["state"] = state;
    
    String message;
    serializeJson(doc, message);
    
    // Send event
    events->send(message.c_str(), "state", millis());
}
