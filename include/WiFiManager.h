#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "credentials.h"

class WiFiManager {
private:
    // FreeRTOS event group to signal WiFi events
    EventGroupHandle_t wifiEventGroup;
    
    // Event bits for WiFi events
    static const int WIFI_CONNECTED_BIT = BIT0;
    static const int WIFI_FAIL_BIT = BIT1;
    
    // WiFi credentials
    const char* ssid;
    const char* password;
    const char* hostname;
    
    // Soft AP credentials
    const char* softApSsid;
    const char* softApPassword;
    int channel;
    bool hideSsid;
    int maxConnections;
    
    // Static IP configuration
    IPAddress localIp;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress primaryDns;
    IPAddress secondaryDns;
    
    // WiFi event handlers
    static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    
    // Task handle for WiFi monitoring task
    TaskHandle_t wifiMonitorTaskHandle;
    
    // WiFi monitoring task
    static void wifiMonitorTask(void* parameter);
    
    // Flag to indicate if static IP is configured
    bool useStaticIp;
    
    // Flag to indicate if Soft AP is enabled
    bool softApEnabled;
    
    // Flag to indicate if WiFi is connected
    bool isConnected;
    
public:
    WiFiManager();
    
    // Initialize WiFi
    void begin(const char* ssid, const char* password, const char* hostname = "esp32");
    
    // Configure static IP
    void configureStaticIp(IPAddress localIp, IPAddress gateway, IPAddress subnet, 
                          IPAddress primaryDns = IPAddress(8, 8, 8, 8), 
                          IPAddress secondaryDns = IPAddress(8, 8, 4, 4));
    
    // Configure Soft AP
    void configureSoftAp(const char* ssid, const char* password = nullptr, 
                        int channel = 1, bool hideSsid = false, int maxConnections = 4);
    
    // Start Soft AP
    void startSoftAp();
    
    // Stop Soft AP
    void stopSoftAp();
    
    // Get WiFi status
    bool isWiFiConnected();
    
    // Get local IP address
    IPAddress getLocalIp();
    
    // Get Soft AP IP address
    IPAddress getSoftApIp();
    
    // Get RSSI
    int getRssi();
    
    // Get MAC address
    String getMacAddress();
    
    // Get hostname
    String getHostname();
};

#endif // WIFI_MANAGER_H
