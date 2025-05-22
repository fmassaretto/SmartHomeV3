#include <Arduino.h>
#include "WiFiManager.h"
#include "UserManager.h"
#include "DeviceManager.h"
#include "SessionManager.h"
#include "WebServer.h"
#include "OtaManager.h"
#include "AlexaManager.h"
#include "credentials.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Global objects
WiFiManager wifiManager;
UserManager userManager;
DeviceManager deviceManager;
WebServer* webServer;
OtaManager* otaManager;
AlexaManager* alexaManager;

// Task handles
TaskHandle_t taskButtonsHandle;

// Task to check physical buttons
void taskButtons(void* parameter) {
  for (;;) {
    deviceManager.checkInputs();
    vTaskDelay(pdMS_TO_TICKS(30)); // debounce and CPU friendly
  }
}

void setup() {
  // Initialize serial
  Serial.begin(115200);
  Serial.println("\n\nESP32 Smart Home System Starting...");
  
  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("Failed to mount LittleFS");
  }
  
  // Initialize WiFi
  wifiManager.begin(WIFI_SSID, WIFI_PASSWORD, "esp32-smart-home");
  
  // Configure static IP if needed
  IPAddress localIP(192, 168, 0, 222);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);
  IPAddress secondaryDNS(8, 8, 4, 4);
  wifiManager.configureStaticIp(localIP, gateway, subnet, primaryDNS, secondaryDNS);
  
  // Configure and start Soft AP
  wifiManager.configureSoftAp(SOFT_AP_SSID, SOFT_AP_PASSWORD, 10, false, 2);
  wifiManager.startSoftAp();
  
  // Initialize user manager
  if (!userManager.begin()) {
    Serial.println("Failed to initialize user manager");
  }
  
  // Initialize device manager
  if (!deviceManager.begin()) {
    Serial.println("Failed to initialize device manager");
  }
  
  // Create web server
  webServer = new WebServer(&wifiManager, &userManager, &deviceManager);
  webServer->begin();
  
  // Create OTA manager
  AsyncWebServer* server = webServer->getServer();
  otaManager = new OtaManager(server);
  otaManager->begin();
  otaManager->setEnabled(true);
  
  // Create Alexa manager
  alexaManager = new AlexaManager(&deviceManager);
  alexaManager->begin();
  
  // Create button checking task
  xTaskCreatePinnedToCore(
    taskButtons,
    "TaskButtons",
    4096,
    NULL,
    1,
    &taskButtonsHandle,
    1
  );
  
  Serial.println("ESP32 Smart Home System Started");
}

void loop() {
  // Handle Alexa events
  alexaManager->handle();
  
  // Nothing else to do in the main loop as most functionality is handled by tasks and async handlers
  delay(10);
}
