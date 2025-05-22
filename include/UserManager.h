#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <vector>
#include <map>

// User role definitions
enum class UserRole {
    ADMIN,      // Can manage users and control all devices
    OPERATOR,   // Can control assigned devices
    VIEWER      // Can only view device status
};

// User structure
struct User {
    String username;
    String passwordHash;
    UserRole role;
    std::vector<int> allowedDevices;  // List of device channels this user can control
};

class UserManager {
private:
    std::vector<User> users;
    String configFile = "/users.json";
    bool initialized = false;
    
    // Hash a password (simple implementation - in production use a proper hashing algorithm)
    String hashPassword(const String& password);
    
    // Save users to file
    bool saveUsers();
    
    // Load users from file
    bool loadUsers();
    
public:
    UserManager();
    
    // Initialize the user manager
    bool begin();
    
    // Add a new user
    bool addUser(const String& username, const String& password, UserRole role, const std::vector<int>& allowedDevices = {});
    
    // Update an existing user
    bool updateUser(const String& username, const String& password, UserRole role, const std::vector<int>& allowedDevices = {});
    
    // Delete a user
    bool deleteUser(const String& username);
    
    // Authenticate a user
    bool authenticate(const String& username, const String& password);
    
    // Get user role
    UserRole getUserRole(const String& username);
    
    // Check if user can control a device
    bool canControlDevice(const String& username, int deviceChannel);
    
    // Get all users
    std::vector<User> getAllUsers();
    
    // Get user by username
    User* getUser(const String& username);
    
    // Create default admin user if no users exist
    void createDefaultAdminIfNeeded();
};

#endif // USER_MANAGER_H
