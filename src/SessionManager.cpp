#include "../include/SessionManager.h"
#include <random>

// Constructor
SessionManager::SessionManager() {
}

// Generate a random session ID
String SessionManager::generateSessionId() {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int charsetSize = sizeof(charset) - 1;
    
    String sessionId = "";
    for (int i = 0; i < 32; i++) {
        int index = random(0, charsetSize);
        sessionId += charset[index];
    }
    
    return sessionId;
}

// Create a new session
String SessionManager::createSession(const String& username) {
    // Generate a new session ID
    String sessionId = generateSessionId();
    
    // Store session
    sessions[sessionId] = username;
    sessionTimes[sessionId] = millis();
    
    return sessionId;
}

// Validate a session
bool SessionManager::validateSession(const String& sessionId) {
    // Check if session exists
    if (sessions.find(sessionId) == sessions.end()) {
        return false;
    }
    
    // Check if session has expired
    unsigned long currentTime = millis();
    unsigned long sessionTime = sessionTimes[sessionId];
    
    // Handle millis() overflow
    if (currentTime < sessionTime) {
        sessionTime = currentTime;
    }
    
    if (currentTime - sessionTime > sessionTimeout) {
        // Session has expired, delete it
        deleteSession(sessionId);
        return false;
    }
    
    // Update session time
    sessionTimes[sessionId] = currentTime;
    
    return true;
}

// Get username from session
String SessionManager::getUsernameFromSession(const String& sessionId) {
    // Check if session exists
    if (sessions.find(sessionId) == sessions.end()) {
        return "";
    }
    
    return sessions[sessionId];
}

// Delete a session
bool SessionManager::deleteSession(const String& sessionId) {
    // Check if session exists
    if (sessions.find(sessionId) == sessions.end()) {
        return false;
    }
    
    // Remove session
    sessions.erase(sessionId);
    sessionTimes.erase(sessionId);
    
    return true;
}

// Clean expired sessions
void SessionManager::cleanExpiredSessions() {
    unsigned long currentTime = millis();
    
    // Find expired sessions
    std::vector<String> expiredSessions;
    for (const auto& session : sessionTimes) {
        unsigned long sessionTime = session.second;
        
        // Handle millis() overflow
        if (currentTime < sessionTime) {
            sessionTime = currentTime;
        }
        
        if (currentTime - sessionTime > sessionTimeout) {
            expiredSessions.push_back(session.first);
        }
    }
    
    // Delete expired sessions
    for (const String& sessionId : expiredSessions) {
        deleteSession(sessionId);
    }
}

// Middleware for checking authentication
bool SessionManager::authMiddleware(AsyncWebServerRequest *request, UserManager* userManager) {
    // Check if request has session cookie
    if (!request->hasHeader("Cookie")) {
        return false;
    }
    
    // Get session ID from cookie
    String cookie = request->getHeader("Cookie")->value();
    int sessionStart = cookie.indexOf("session=");
    if (sessionStart == -1) {
        return false;
    }
    
    sessionStart += 8;  // Length of "session="
    int sessionEnd = cookie.indexOf(";", sessionStart);
    if (sessionEnd == -1) {
        sessionEnd = cookie.length();
    }
    
    String sessionId = cookie.substring(sessionStart, sessionEnd);
    
    // Validate session
    if (!validateSession(sessionId)) {
        return false;
    }
    
    return true;
}

// Middleware for checking admin role
bool SessionManager::adminMiddleware(AsyncWebServerRequest *request, UserManager* userManager) {
    // First check authentication
    if (!authMiddleware(request, userManager)) {
        return false;
    }
    
    // Get session ID from cookie
    String cookie = request->getHeader("Cookie")->value();
    int sessionStart = cookie.indexOf("session=");
    sessionStart += 8;  // Length of "session="
    int sessionEnd = cookie.indexOf(";", sessionStart);
    if (sessionEnd == -1) {
        sessionEnd = cookie.length();
    }
    
    String sessionId = cookie.substring(sessionStart, sessionEnd);
    
    // Get username from session
    String username = getUsernameFromSession(sessionId);
    
    // Check if user is admin
    return userManager->getUserRole(username) == UserRole::ADMIN;
}

// Middleware for checking device control permission
bool SessionManager::deviceControlMiddleware(AsyncWebServerRequest *request, UserManager* userManager, int deviceChannel) {
    // First check authentication
    if (!authMiddleware(request, userManager)) {
        return false;
    }
    
    // Get session ID from cookie
    String cookie = request->getHeader("Cookie")->value();
    int sessionStart = cookie.indexOf("session=");
    sessionStart += 8;  // Length of "session="
    int sessionEnd = cookie.indexOf(";", sessionStart);
    if (sessionEnd == -1) {
        sessionEnd = cookie.length();
    }
    
    String sessionId = cookie.substring(sessionStart, sessionEnd);
    
    // Get username from session
    String username = getUsernameFromSession(sessionId);
    
    // Check if user can control device
    return userManager->canControlDevice(username, deviceChannel);
}
