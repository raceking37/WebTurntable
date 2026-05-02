#include <Arduino_JSON.h>
#include <JSON.h>
#include <JSONVar.h>
#include "server.h"
#include "turntable.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
JSONVar gPageContent;

namespace
{
	constexpr char SETTINGS_FILE[] = "/turntable_state.bin";
	constexpr uint32_t SETTINGS_MAGIC = 0x5454424C;
	constexpr unsigned long SETTINGS_SAVE_DELAY_MS = 750;

	struct PersistedTurntableState
	{
		uint32_t magic;
		uint8_t direction;
		uint16_t sliderSpeed;
		uint8_t mode;
	};

	bool gSettingsDirty = false;
	unsigned long gLastSettingsChangeMs = 0;

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

	void logTurntableStatus(const char* context)
	{
		Serial.print(context);
		Serial.print(" direction=");
		Serial.print(directionToString(gTurntable.direction));
		Serial.print(", speed=");
		Serial.print(gTurntable.sliderSpeed);
		Serial.print(", mode=");
		Serial.println(modeToString(gTurntable.mode));
	}

	uint16_t clampSliderSpeed(uint16_t sliderSpeed)
	{
		return sliderSpeed > TOTAL_SLIDER_STEPS ? TOTAL_SLIDER_STEPS : sliderSpeed;
	}

	bool isValidDirection(uint8_t direction)
	{
		return direction <= RotateDirection::CCW;
	}

	bool isValidMode(uint8_t mode)
	{
		return mode <= RotateMode::HALF_STEPPING;
	}

	void applyTurntableState(uint8_t direction, uint16_t sliderSpeed, uint8_t mode)
	{
		gTurntable.direction = isValidDirection(direction) ? direction : RotateDirection::OFF;
		gTurntable.sliderSpeed = clampSliderSpeed(sliderSpeed);
		gTurntable.delayValue = MAX_DELAY - (DELAY_MULTIPLIER * gTurntable.sliderSpeed);
		gTurntable.mode = isValidMode(mode) ? mode : RotateMode::HALF_STEPPING;
	}

	void queueTurntableStateSave()
	{
		gSettingsDirty = true;
		gLastSettingsChangeMs = millis();
	}

	void saveTurntableState()
	{
		PersistedTurntableState persistedState = {
			SETTINGS_MAGIC,
			gTurntable.direction,
			gTurntable.sliderSpeed,
			gTurntable.mode
		};

		File settingsFile = LittleFS.open(SETTINGS_FILE, "w");
		if (!settingsFile)
		{
			Serial.println("Failed to open settings file for writing");
			return;
		}

		size_t bytesWritten = settingsFile.write(reinterpret_cast<const uint8_t*>(&persistedState), sizeof(persistedState));
		settingsFile.close();

		if (bytesWritten != sizeof(persistedState))
		{
			Serial.println("Failed to persist turntable settings");
			return;
		}

		gSettingsDirty = false;
		logTurntableStatus("Saved state:");
	}

	void loadTurntableState()
	{
		applyTurntableState(gTurntable.direction, gTurntable.sliderSpeed, gTurntable.mode);

		if (!LittleFS.exists(SETTINGS_FILE))
		{
			return;
		}

		File settingsFile = LittleFS.open(SETTINGS_FILE, "r");
		if (!settingsFile)
		{
			Serial.println("Failed to open settings file for reading");
			return;
		}

		PersistedTurntableState persistedState;
		size_t bytesRead = settingsFile.readBytes(reinterpret_cast<char*>(&persistedState), sizeof(persistedState));
		settingsFile.close();

		if (bytesRead != sizeof(persistedState) || persistedState.magic != SETTINGS_MAGIC)
		{
			Serial.println("Ignoring invalid turntable settings");
			return;
		}

		applyTurntableState(persistedState.direction, persistedState.sliderSpeed, persistedState.mode);
	}
}

void onWsEvent (AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	switch (type)
	{
	case WS_EVT_CONNECT:
		Serial.println("Client connected");
		logTurntableStatus("Current state:");
		client->text(JSON.stringify(gPageContent));
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

void handleClientMessage(void *arg, uint8_t *data, size_t len) {
	AwsFrameInfo *info = (AwsFrameInfo*)arg;
	if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
	
		char jsonString[len + 1];
		memcpy(jsonString, data, len);
		jsonString[len] = '\0';\

		JSONVar jsonData = JSON.parse(jsonString);

		String msgType = (const char*)jsonData["type"];
		
		if (msgType == "stepping_direction")
		{
			uint8_t direction = (int)jsonData["content"];
			if (isValidDirection(direction) && gTurntable.direction != direction)
			{
				gTurntable.direction = direction;
				queueTurntableStateSave();
			}
			updatePage();
		}
		else if (msgType == "stepping_speed")
		{
			uint16_t sliderSpeed = (int)jsonData["content"];
			sliderSpeed = clampSliderSpeed(sliderSpeed);

			if (gTurntable.sliderSpeed != sliderSpeed)
			{
				gTurntable.sliderSpeed = sliderSpeed;
				gTurntable.delayValue = MAX_DELAY - (DELAY_MULTIPLIER * gTurntable.sliderSpeed);
				queueTurntableStateSave();
			}
			updatePage();
		}
		else if (msgType == "stepping_mode")
		{
			uint8_t mode = (int)jsonData["content"];
			if (isValidMode(mode) && gTurntable.mode != mode)
			{
				gTurntable.mode = mode;
				queueTurntableStateSave();
			}
			updatePage();
		}
	}
}

void initWebserver()
{
	if (!LittleFS.begin())
	{
		Serial.println("Error mounting FS");
		while(1);
	}

	loadTurntableState();

	// Setup the access point
	WiFi.softAP(SSID, PASSWORD);
	Serial.println(WiFi.softAPIP());

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
	
	server.on("/", HTTP_GET, onIndexRequest);
  	server.on("/style.css", HTTP_GET, onCSSRequest);

	server.serveStatic("/", LittleFS, "/");
	server.begin();

	gPageContent["stepping_direction"] = gTurntable.direction;
	gPageContent["stepping_speed"] = gTurntable.sliderSpeed;
	gPageContent["stepping_mode"] = gTurntable.mode;
}

void handleWebserver()
{
	if (!gSettingsDirty)
	{
		return;
	}

	if (millis() - gLastSettingsChangeMs < SETTINGS_SAVE_DELAY_MS)
	{
		return;
	}

	saveTurntableState();
}

// Send webpage
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

// Update the connected clients
void updatePage()
{
	gPageContent["stepping_direction"] = gTurntable.direction;
	gPageContent["stepping_speed"] = gTurntable.sliderSpeed;
	gPageContent["stepping_mode"] = gTurntable.mode;
	
	String jsonString = JSON.stringify(gPageContent);
	ws.textAll(jsonString);
}