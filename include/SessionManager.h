#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <Arduino.h>
#include <map>
#include <ESPAsyncWebServer.h>
#include "UserManager.h"

class SessionManager {
private:
    std::map<String, String> sessions;  // session_id -> username
    std::map<String, unsigned long> sessionTimes;  // session_id -> last_activity_time
    unsigned long sessionTimeout = 3600000;  // 1 hour in milliseconds
    
    // Generate a random session ID
    String generateSessionId();
    
public:
    SessionManager();
    
    // Create a new session
    String createSession(const String& username);
    
    // Validate a session
    bool validateSession(const String& sessionId);
    
    // Get username from session
    String getUsernameFromSession(const String& sessionId);
    
    // Delete a session
    bool deleteSession(const String& sessionId);
    
    // Clean expired sessions
    void cleanExpiredSessions();
    
    // Middleware for checking authentication
    bool authMiddleware(AsyncWebServerRequest *request, UserManager* userManager);
    
    // Middleware for checking admin role
    bool adminMiddleware(AsyncWebServerRequest *request, UserManager* userManager);
    
    // Middleware for checking device control permission
    bool deviceControlMiddleware(AsyncWebServerRequest *request, UserManager* userManager, int deviceChannel);
};

#endif // SESSION_MANAGER_H
