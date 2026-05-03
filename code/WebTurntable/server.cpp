#include "server.h"
#include "turntable.h"

#include <Arduino_JSON.h>
#include <LittleFS.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

namespace
{
	AsyncWebServer server(80);
	AsyncWebSocket ws("/ws");
	JSONVar pageContent;

	constexpr char SETTINGS_FILE[] = "/turntable_state.bin";
	constexpr unsigned long SETTINGS_SAVE_DELAY_MS = 750;

	bool settingsDirty = false;
	unsigned long lastSettingsChangeMs = 0;

	// Error handler that halts execution and blinks the LED indefinitely
    void haltWithErrorIndicator()
    {
        Serial.println("Critical error: Halting execution");
        while (true)
        {
            digitalWrite(LED_PIN, HIGH);
            delay(500);
            digitalWrite(LED_PIN, LOW);
            delay(500);
        }
    }

	// Helper to convert rotation direction enum to string for logging
    const char* directionToString(uint8_t direction)
    {
        switch (direction)
        {
            case RotateDirection::CW:
                return "CW";
            case RotateDirection::OFF:
                return "OFF";
            case RotateDirection::CCW:
                return "CCW";
            default:
                return "UNKNOWN";
        }
    }

	// Helper to convert stepping mode enum to string for logging
    const char* modeToString(uint8_t mode)
    {
        switch (mode)
        {
            case RotateMode::FULL_STEPPING:
                return "FULL";
            case RotateMode::HALF_STEPPING:
                return "HALF";
            default:
                return "UNKNOWN";
        }
    }

	// Log information about the current turntable state to serial console
    void logTurntableStatus(const char* context)
    {
        Serial.print(context);
        Serial.print(" direction=");
        Serial.print(directionToString(g_turntable.direction));
        Serial.print(", speed=");
        Serial.print(g_turntable.sliderSpeed);
        Serial.print(", mode=");
        Serial.println(modeToString(g_turntable.mode));
    }

	// Mark settings as dirty and start the save delay timer
    void queueSettingsSave()
    {
        settingsDirty = true;
        lastSettingsChangeMs = millis();
    }

	// Save the current turntable settings to flash memory
    void saveTurntableState()
    {
        File settingsFile = LittleFS.open(SETTINGS_FILE, "w");
        if (!settingsFile)
        {
            Serial.println("Failed to open settings file for writing");
            return;
        }

        size_t bytesWritten = settingsFile.write(reinterpret_cast<const uint8_t*>(&g_turntable), sizeof(g_turntable));
        settingsFile.close();

        if (bytesWritten != sizeof(g_turntable))
        {
            Serial.println("Error: Incomplete write to settings file");
            return;
        }

        settingsDirty = false;
        logTurntableStatus("Settings saved:");
    }

	// Load turntable settings from flash memory, returns true if successful
    bool loadTurntableState()
    {
        if (!LittleFS.exists(SETTINGS_FILE))
        {
            Serial.println("No saved settings file found");
            return false;
        }

        File settingsFile = LittleFS.open(SETTINGS_FILE, "r");
        if (!settingsFile)
        {
            Serial.println("Error: Failed to open settings file for reading");
            return false;
        }

        TurntableSettings loadedSettings;
        size_t bytesRead = settingsFile.readBytes(reinterpret_cast<char*>(&loadedSettings), sizeof(loadedSettings));
        settingsFile.close();

        // Validate magic number and file integrity
        if (bytesRead != sizeof(loadedSettings) || loadedSettings.magic != TurntableSettings().MAGIC)
        {
            Serial.println("Error: Settings file invalid");
            return false;
        }

        // Apply loaded settings
        g_turntable = loadedSettings;
        logTurntableStatus("Settings loaded:");
        return true;
    }

	// Update the connected clients
	void updatePage()
	{
		pageContent["stepping_direction"] = g_turntable.direction;
		pageContent["stepping_speed"] = g_turntable.sliderSpeed;
		pageContent["stepping_mode"] = g_turntable.mode;
		
		String jsonString = JSON.stringify(pageContent);
		ws.textAll(jsonString);
	}

	// Handle incoming WebSocket messages from clients
	void handleClientMessage(void *arg, uint8_t *data, size_t len) 
	{
		AwsFrameInfo *info = (AwsFrameInfo*)arg;
		if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
		{
			JSONVar jsonData = JSON.parse((char*)data);
			String msgType = (const char*)jsonData["type"];
			
			// Handle stepping direction change
			if (msgType == "stepping_direction")
			{
				uint8_t direction = (uint8_t)jsonData["content"];
				if ((direction <= RotateDirection::CCW) && (g_turntable.direction != direction))
				{
					g_turntable.direction = (RotateDirection) direction;
					queueSettingsSave();
				}
				updatePage();
			}
			// Handle speed/slider change
			else if (msgType == "stepping_speed")
			{
				uint16_t sliderSpeed = (uint16_t)jsonData["content"];
				sliderSpeed = sliderSpeed > TOTAL_SLIDER_STEPS ? TOTAL_SLIDER_STEPS : sliderSpeed;

				if (g_turntable.sliderSpeed != sliderSpeed)
				{
					g_turntable.sliderSpeed = sliderSpeed;
					queueSettingsSave();
				}
				updatePage();
			}
			// Handle stepping mode change
			else if (msgType == "stepping_mode")
			{
				uint8_t mode = (uint8_t)jsonData["content"];
				if ((mode <= RotateMode::HALF_STEPPING) && (g_turntable.mode != mode))
				{
					g_turntable.mode = (RotateMode) mode;
					queueSettingsSave();
				}
				updatePage();
			}
		}
	}

	// WebSocket event handler
	void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
	{
		switch (type)
		{
		case WS_EVT_CONNECT:
			Serial.println("Client connected");
			logTurntableStatus("Current state:");
			client->text(JSON.stringify(pageContent));
			break;

		case WS_EVT_DISCONNECT:
			Serial.println("Client disconnected");
			break;

		case WS_EVT_DATA:
			handleClientMessage(arg, data, len);
			break;

		default:
			break;
		}
	}

	// Send main page HTML
	void onIndexRequest(AsyncWebServerRequest *request) 
	{
		IPAddress remote_ip = request->client()->remoteIP();
		Serial.println("[" + remote_ip.toString() + "] HTTP GET request of " + request->url());
		request->send(LittleFS, "/index.html", "text/html");
	}

	// Send style sheet
	void onCSSRequest(AsyncWebServerRequest *request) 
	{
		IPAddress remote_ip = request->client()->remoteIP();
		Serial.println("[" + remote_ip.toString() + "] HTTP GET request of " + request->url());
		request->send(LittleFS, "/style.css", "text/css");
	}
}

// Initialize the web server and load saved settings from flash
void initWebserver()
{
	// Initialize file system
	if (!LittleFS.begin() || !LittleFS.exists("/index.html"))
	{
		Serial.println("Error: Failed to mount LittleFS");
		haltWithErrorIndicator();
	}

	// Load persisted settings from flash
	loadTurntableState();

	// Setup WiFi access point
	WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
	Serial.print("WiFi AP started - IP: ");
	Serial.println(WiFi.softAPIP());

	// Configure WebSocket
	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
	
	// Configure HTTP routes
	server.on("/", HTTP_GET, onIndexRequest);
	server.on("/style.css", HTTP_GET, onCSSRequest);

	// Serve static files from LittleFS
	server.serveStatic("/", LittleFS, "/");
	server.begin();

	// Initialize page content
	updatePage();
}

// Handle settings storage with delayed write to flash to minimize wear
void handleSettingsStorage()
{
	if (settingsDirty && millis() - lastSettingsChangeMs >= SETTINGS_SAVE_DELAY_MS)
	{
		saveTurntableState();
	}
}