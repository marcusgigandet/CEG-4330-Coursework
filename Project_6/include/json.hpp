#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

class Json
{
	WiFiServer m_json_server;
	WiFiClient m_json_client;
	JsonDocument m_json;
	SemaphoreHandle_t m_mutex;

	void start_json_server(int port);
	void handle_json_from_client();

public:
	explicit Json(SemaphoreHandle_t& mutex, int port) : m_mutex(mutex) { start_json_server(port); }

    static void handle_json_from_client_task(void* parameter);

    bool send_json_to_client();

	void update_json(uint32_t press_count, uint32_t press_duration);

	[[nodiscard]] String to_string() const;

	SemaphoreHandle_t& mutex() { return m_mutex; };
};
