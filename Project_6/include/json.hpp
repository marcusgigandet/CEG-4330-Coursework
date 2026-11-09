#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wifi.h>

class Json
{
	WiFiServer m_json_server;
	WiFiClient m_json_client;
	JsonDocument m_json;

	void start_json_server(int port);

public:
	Json() = default;
	Json(int port) { start_json_server(port); }

    static void handle_json_from_client_task(void* parameter);

	void update_json();
};
