#include "json.hpp"
#include "board.hpp"
#include "freertos/idf_additions.h"
#include "portmacro.h"

#include <cmath>

void Json::handle_json_from_client_task(void* parameter)
{
	auto* json = static_cast<Json*>(parameter);

	while (true)
	{
		if (0 == json->m_json_client.connected())
		{
			xSemaphoreTake(json->mutex(), portMAX_DELAY);
			{ // Critical section
				json->m_json_client = json->m_json_server.accept();

				if (1 == json->m_json_client.connected())
				{
					// Try to send json if client is connected
					if (json->send_json_to_client())
					{
						json->handle_json_from_client();
					}
				}
			}
			xSemaphoreGive(json->mutex());
		}

		vTaskDelay(1);
	}
}

bool Json::send_json_to_client()
{
	String json_str;

	// Send serialized json over WiFi
	if (serializeJson(m_json, json_str) > 0)
	{
		m_json_client.println(json_str);
		return true;
	}

	// An error occurred during serialization
	return false;
}

void Json::handle_json_from_client()
{
	if (0 == m_json_client.available())
	{
		return;
	}

	String received_data = m_json_client.readStringUntil('\n');
	received_data.trim();
	JsonDocument rx_json;

	// Return if errors in deserialized json
	if (deserializeJson(rx_json, received_data))
	{
		return;
	}

	// Return if invalid duty cycle or frequency
	const JsonVariantConst FREQUENCY  = rx_json["frequency"];
	const JsonVariantConst DUTY_CYCLE = rx_json["dutyCycle"];
	if (!FREQUENCY.is<float>() || !DUTY_CYCLE.is<float>())
	{
		return;
	}

	// Convert floats to unsigned ints for processing
	const float FREQUENCY_HZ = constrain(FREQUENCY.as<float>(), MIN_FREQUENCY_HZ, MAX_FREQUENCY_HZ);
	const float DUTY_PERCENT = constrain(DUTY_CYCLE.as<float>(), 0.0f, 100.0f);
	const uint32_t FREQ_HZ	 = static_cast<uint32_t>(std::round(FREQUENCY_HZ));
	const uint32_t DUTY		 = static_cast<uint32_t>(std::round(DUTY_PERCENT / 100.0f * MAX_DUTY));

	// Log json
	Serial.printf(
		"Processed Json %u, %u\n",
		static_cast<unsigned>(FREQ_HZ),
		static_cast<unsigned>(DUTY));

	// Update frequency
	ledcChangeFrequency(pins::LED, FREQ_HZ, PWM_RESOLUTION);

	// Update PWM
	// The LED is active low, so invert the PWM duty
	ledcWrite(pins::LED, MAX_DUTY - DUTY);
}

void Json::start_json_server(int port)
{
	// Return early on no WiFi connection
	if (WiFi.status() != WL_CONNECTED)
	{
		return;
	}

	// Start server
	m_json_server = WiFiServer(port);
	m_json_server.begin();
}

void Json::update_json(uint32_t press_count, uint32_t press_duration)
{
	m_json["buttonPresses"]	  = press_count;
	m_json["buttonPressTime"] = press_duration;
}

String Json::to_string() const
{
	String json_string;

	(void)serializeJson(m_json, json_string);

	return json_string;
}
