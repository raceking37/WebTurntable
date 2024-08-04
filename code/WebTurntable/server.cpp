#include <Arduino_JSON.h>
#include <JSON.h>
#include <JSONVar.h>
#include "server.h"
#include "turntable.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
JSONVar gPageContent;

void onWsEvent (AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	switch (type)
	{
	case WS_EVT_CONNECT:
		Serial.println("Client connected");
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

		String msgType = jsonData["type"];
		
		if (msgType == "stepping_direction")
		{
			gTurntable.direction = jsonData["content"];
			updatePage();
		}
		else if (msgType == "stepping_speed")
		{
			gTurntable.sliderSpeed = jsonData["content"];
			gTurntable.delayValue = MAX_DELAY - (DELAY_MULTIPLIER * gTurntable.sliderSpeed);
			updatePage();
		}
		else if (msgType == "stepping_mode")
		{
			gTurntable.mode = jsonData["content"];
			updatePage();
		}
	}
}

void initWebserver()
{
	if (!SPIFFS.begin())
	{
		Serial.println("Error mounting FS");
		while(1);
	}

	// Setup the access point
	WiFi.softAP(SSID, PASSWORD);
	Serial.println(WiFi.softAPIP());

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
	
	server.on("/", HTTP_GET, onIndexRequest);
  	server.on("/style.css", HTTP_GET, onCSSRequest);

	server.serveStatic("/", SPIFFS, "/");
	server.begin();

	gPageContent["stepping_direction"] = gTurntable.direction;
	gPageContent["stepping_speed"] = gTurntable.sliderSpeed;
	gPageContent["stepping_mode"] = gTurntable.mode;
}

// Send webpage
void onIndexRequest(AsyncWebServerRequest *request) 
{
	IPAddress remote_ip = request->client()->remoteIP();
	Serial.println("[" + remote_ip.toString() + "] HTTP GET request of " + request->url());
	request->send(SPIFFS, "/index.html", "text/html");
}


// Send style sheet
void onCSSRequest(AsyncWebServerRequest *request) 
{
	IPAddress remote_ip = request->client()->remoteIP();
	Serial.println("[" + remote_ip.toString() + "] HTTP GET request of " + request->url());
	request->send(SPIFFS, "/style.css", "text/css");
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