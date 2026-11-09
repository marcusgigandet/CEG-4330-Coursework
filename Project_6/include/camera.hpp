#pragma once

#include "WiFiServer.h"
#include <ArduinoJson.h>
#include <WiFi.h>

class Camera
{
	WiFiServer m_camera_server;
	WiFiClient m_camera_client;

	void start_server(int port);

public:
	Camera() = default;
	Camera(int port) { start_server(port); }

    static void send_image_to_client_task(void* parameter);

	bool send_image_to_client(WiFiClient& client);

	WiFiServer* server() { return &m_camera_server; }
};
