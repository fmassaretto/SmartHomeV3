#include "../include/WiFiManager.h"

// Constructor
WiFiManager::WiFiManager() : 
    ssid(nullptr), 
    password(nullptr), 
    hostname("esp32"),
    softApSsid(nullptr),
    softApPassword(nullptr),
    channel(1),
    hideSsid(false),
    maxConnections(4),
    useStaticIp(false),
    softApEnabled(false),
    isConnected(false)
{
    // Create the event group
    wifiEventGroup = xEventGroupCreate();
}

// Initialize WiFi
void WiFiManager::begin(const char* ssid, const char* password, const char* hostname) {
    this->ssid = ssid;
    this->password = password;
    this->hostname = hostname;
    
    // Set WiFi mode to station
    WiFi.mode(WIFI_STA);
    
    // Set hostname
    WiFi.setHostname(hostname);
    
    // Register event handlers
    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        switch (event) {
            case SYSTEM_EVENT_STA_START:
                Serial.println("WiFi station started");
                break;
            case SYSTEM_EVENT_STA_CONNECTED:
                Serial.println("WiFi connected");
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                Serial.print("WiFi got IP: ");
                Serial.println(WiFi.localIP());
                isConnected = true;
                xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                Serial.println("WiFi disconnected, attempting to reconnect...");
                isConnected = false;
                xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
                WiFi.reconnect();
                break;
            default:
                break;
        }
    });
    
    // Configure static IP if needed
    if (useStaticIp) {
        if (!WiFi.config(localIp, gateway, subnet, primaryDns, secondaryDns)) {
            Serial.println("WiFi static IP configuration failed");
        }
    }
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi...");
    
    // Wait for connection or timeout
    EventBits_t bits = xEventGroupWaitBits(
        wifiEventGroup,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        Serial.println("Connected to WiFi");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else if (bits & WIFI_FAIL_BIT) {
        Serial.println("Failed to connect to WiFi");
    } else {
        Serial.println("UNEXPECTED EVENT");
    }
    
    // Start WiFi monitoring task
    xTaskCreatePinnedToCore(
        wifiMonitorTask,
        "WiFiMonitorTask",
        4096,
        this,
        1,
        &wifiMonitorTaskHandle,
        0);
    
    // Start mDNS responder
    if (MDNS.begin(hostname)) {
        Serial.println("mDNS responder started");
        // Add service to mDNS
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("Error setting up mDNS responder");
    }
}

// Configure static IP
void WiFiManager::configureStaticIp(IPAddress localIp, IPAddress gateway, IPAddress subnet, 
                                   IPAddress primaryDns, IPAddress secondaryDns) {
    this->localIp = localIp;
    this->gateway = gateway;
    this->subnet = subnet;
    this->primaryDns = primaryDns;
    this->secondaryDns = secondaryDns;
    this->useStaticIp = true;
}

// Configure Soft AP
void WiFiManager::configureSoftAp(const char* ssid, const char* password, 
                                 int channel, bool hideSsid, int maxConnections) {
    this->softApSsid = ssid;
    this->softApPassword = password;
    this->channel = channel;
    this->hideSsid = hideSsid;
    this->maxConnections = maxConnections;
}

// Start Soft AP
void WiFiManager::startSoftAp() {
    if (softApSsid == nullptr) {
        Serial.println("Soft AP SSID not configured");
        return;
    }
    
    // Set WiFi mode to station + AP
    WiFi.mode(WIFI_AP_STA);
    
    // Start Soft AP
    if (WiFi.softAP(softApSsid, softApPassword, channel, hideSsid, maxConnections)) {
        Serial.print("Soft AP started with SSID: ");
        Serial.println(softApSsid);
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
        softApEnabled = true;
    } else {
        Serial.println("Failed to start Soft AP");
    }
}

// Stop Soft AP
void WiFiManager::stopSoftAp() {
    if (softApEnabled) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        softApEnabled = false;
        Serial.println("Soft AP stopped");
    }
}

// Get WiFi status
bool WiFiManager::isWiFiConnected() {
    return isConnected;
}

// Get local IP address
IPAddress WiFiManager::getLocalIp() {
    return WiFi.localIP();
}

// Get Soft AP IP address
IPAddress WiFiManager::getSoftApIp() {
    return WiFi.softAPIP();
}

// Get RSSI
int WiFiManager::getRssi() {
    return WiFi.RSSI();
}

// Get MAC address
String WiFiManager::getMacAddress() {
    return WiFi.macAddress();
}

// Get hostname
String WiFiManager::getHostname() {
    return WiFi.getHostname();
}

// WiFi monitoring task
void WiFiManager::wifiMonitorTask(void* parameter) {
    WiFiManager* wifiManager = static_cast<WiFiManager*>(parameter);
    
    for (;;) {
        // Check WiFi connection status
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi connection lost, attempting to reconnect...");
            WiFi.disconnect();
            WiFi.reconnect();
            
            // Wait for connection or timeout
            EventBits_t bits = xEventGroupWaitBits(
                wifiManager->wifiEventGroup,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE,
                pdFALSE,
                10000 / portTICK_PERIOD_MS);
            
            if (bits & WIFI_CONNECTED_BIT) {
                Serial.println("Reconnected to WiFi");
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());
            } else if (bits & WIFI_FAIL_BIT) {
                Serial.println("Failed to reconnect to WiFi");
            }
        }
        
        // Check every 30 seconds
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}
