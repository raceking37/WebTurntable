#ifndef SERVER_H
#define SERVER_H

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>

#define SSID        "TURNTABLE"
#define PASSWORD    ""

void onWsEvent (AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void handleClientMessage(void *arg, uint8_t *data, size_t len);
void initWebserver();
void onIndexRequest(AsyncWebServerRequest *request);
void onCSSRequest(AsyncWebServerRequest *request);
void updatePage();

#endif // SERVER_H