#include "../include/UserManager.h"
#include <FS.h>
#include "../include/credentials.h"

// Constructor
UserManager::UserManager() {
    initialized = false;
}

// Initialize the user manager
bool UserManager::begin() {
    // Initialize LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    
    // Load users from file
    if (!loadUsers()) {
        Serial.println("Failed to load users, creating default configuration");
        // Create default admin user
        createDefaultAdminIfNeeded();
        // Save users to file
        saveUsers();
    }
    
    initialized = true;
    return true;
}

// Hash a password (simple implementation - in production use a proper hashing algorithm)
String UserManager::hashPassword(const String& password) {
    // This is a simple hash function for demonstration purposes
    // In a production environment, use a proper cryptographic hash function
    String hash = "";
    for (unsigned int i = 0; i < password.length(); i++) {
        hash += String((int)password[i] * 13, 16);
    }
    return hash;
}

// Save users to file
bool UserManager::saveUsers() {
    // Create a JSON document
    DynamicJsonDocument doc(4096);
    JsonArray usersArray = doc.createNestedArray("users");
    
    // Add each user to the JSON document
    for (const User& user : users) {
        JsonObject userObj = usersArray.createNestedObject();
        userObj["username"] = user.username;
        userObj["passwordHash"] = user.passwordHash;
        userObj["role"] = static_cast<int>(user.role);
        
        // Add allowed devices
        JsonArray devicesArray = userObj.createNestedArray("allowedDevices");
        for (int deviceChannel : user.allowedDevices) {
            devicesArray.add(deviceChannel);
        }
    }
    
    // Open the file for writing
    File file = LittleFS.open(configFile, "w");
    if (!file) {
        Serial.println("Failed to open users file for writing");
        return false;
    }
    
    // Serialize JSON to file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write users to file");
        file.close();
        return false;
    }
    
    file.close();
    return true;
}

// Load users from file
bool UserManager::loadUsers() {
    // Check if file exists
    if (!LittleFS.exists(configFile)) {
        Serial.println("Users file does not exist");
        return false;
    }
    
    // Open the file for reading
    File file = LittleFS.open(configFile, "r");
    if (!file) {
        Serial.println("Failed to open users file for reading");
        return false;
    }
    
    // Parse JSON
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.print("Failed to parse users file: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Clear existing users
    users.clear();
    
    // Read users from JSON
    JsonArray usersArray = doc["users"];
    for (JsonObject userObj : usersArray) {
        User user;
        user.username = userObj["username"].as<String>();
        user.passwordHash = userObj["passwordHash"].as<String>();
        user.role = static_cast<UserRole>(userObj["role"].as<int>());
        
        // Read allowed devices
        JsonArray devicesArray = userObj["allowedDevices"];
        for (int deviceChannel : devicesArray) {
            user.allowedDevices.push_back(deviceChannel);
        }
        
        users.push_back(user);
    }
    
    return true;
}

// Add a new user
bool UserManager::addUser(const String& username, const String& password, UserRole role, const std::vector<int>& allowedDevices) {
    // Check if user already exists
    for (const User& user : users) {
        if (user.username == username) {
            return false;
        }
    }
    
    // Create new user
    User newUser;
    newUser.username = username;
    newUser.passwordHash = hashPassword(password);
    newUser.role = role;
    newUser.allowedDevices = allowedDevices;
    
    // Add user to list
    users.push_back(newUser);
    
    // Save users to file
    return saveUsers();
}

// Update an existing user
bool UserManager::updateUser(const String& username, const String& password, UserRole role, const std::vector<int>& allowedDevices) {
    // Find user
    for (User& user : users) {
        if (user.username == username) {
            // Update user
            if (password.length() > 0) {
                user.passwordHash = hashPassword(password);
            }
            user.role = role;
            user.allowedDevices = allowedDevices;
            
            // Save users to file
            return saveUsers();
        }
    }
    
    return false;
}

// Delete a user
bool UserManager::deleteUser(const String& username) {
    // Find user
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->username == username) {
            // Remove user
            users.erase(it);
            
            // Save users to file
            return saveUsers();
        }
    }
    
    return false;
}

// Authenticate a user
bool UserManager::authenticate(const String& username, const String& password) {
    // Find user
    for (const User& user : users) {
        if (user.username == username) {
            // Check password
            return user.passwordHash == hashPassword(password);
        }
    }
    
    return false;
}

// Get user role
UserRole UserManager::getUserRole(const String& username) {
    // Find user
    for (const User& user : users) {
        if (user.username == username) {
            return user.role;
        }
    }
    
    // Default to viewer if user not found
    return UserRole::VIEWER;
}

// Check if user can control a device
bool UserManager::canControlDevice(const String& username, int deviceChannel) {
    // Find user
    for (const User& user : users) {
        if (user.username == username) {
            // Admins can control all devices
            if (user.role == UserRole::ADMIN) {
                return true;
            }
            
            // Operators can control assigned devices
            if (user.role == UserRole::OPERATOR) {
                for (int allowedChannel : user.allowedDevices) {
                    if (allowedChannel == deviceChannel) {
                        return true;
                    }
                }
            }
            
            // Viewers cannot control any devices
            return false;
        }
    }
    
    return false;
}

// Get all users
std::vector<User> UserManager::getAllUsers() {
    return users;
}

// Get user by username
User* UserManager::getUser(const String& username) {
    for (User& user : users) {
        if (user.username == username) {
            return &user;
        }
    }
    
    return nullptr;
}

// Create default admin user if no users exist
void UserManager::createDefaultAdminIfNeeded() {
    if (users.empty()) {
        User adminUser;
        adminUser.username = DEFAULT_ADMIN_USER;
        adminUser.passwordHash = hashPassword(DEFAULT_ADMIN_PASSWORD);
        adminUser.role = UserRole::ADMIN;
        
        users.push_back(adminUser);
        
        Serial.println("Created default admin user");
    }
}
