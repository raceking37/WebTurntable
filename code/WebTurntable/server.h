#ifndef SERVER_H
#define SERVER_H

// Access point credentials
constexpr const char* WIFI_SSID       = "TURNTABLE";
constexpr const char* WIFI_PASSWORD   = "";

// Initialize the webserver
void initWebserver();

// Save turntable settings to flash if needed
void handleSettingsStorage();

#endif // SERVER_H