#include "json.hpp"
#include "board.hpp"

#include <cmath>

void Json::handle_json_from_client_task(void* parameter)
{
	auto* json = static_cast<Json*>(parameter);

	while (true)
	{
		if (0 == json->m_json_client.connected())
		{
			json->m_json_client = json->m_json_server.accept();
		}

		if (1 == json->m_json_client.connected())
		{
			// Try to send json if client is connected
			if (json->send_json_to_client(json->m_json_client))
			{
				json->handle_json_from_client(json->m_json_client);
			}
		}

		vTaskDelay(1);
	}
}

bool Json::send_json_to_client(WiFiClient& client)
{
	String json_str;

	// Send serialized json over WiFi
	if (serializeJson(m_json, json_str) > 0)
	{
		client.println(json_str);
		return true;
	}

	// An error occurred during serialization
	return false;
}

void Json::handle_json_from_client(WiFiClient& client)
{
	if (0 == client.available())
	{
		return;
	}

	String received_data = client.readStringUntil('\n');
	received_data.trim();
	JsonDocument rx_json;

	// Return if errors in deserialized json
	if (deserializeJson(rx_json, received_data))
	{
		return;
	}

	// Return if invalid duty cycle or frequency
	const JsonVariantConst frequency = rx_json["frequency"];
	const JsonVariantConst duty_cycle = rx_json["dutyCycle"];
	if (!frequency.is<float>() || !duty_cycle.is<float>())
	{
		return;
	}

	// Convert floats to unsigned ints for processing
	const float frequency_hz = constrain(frequency.as<float>(), MIN_FREQUENCY_HZ, MAX_FREQUENCY_HZ);
	const float duty_percent = constrain(duty_cycle.as<float>(), 0.0f, 100.0f);
	const uint32_t freq_hz = static_cast<uint32_t>(std::round(frequency_hz));
	const uint32_t duty = static_cast<uint32_t>(std::round(duty_percent / 100.0f * MAX_DUTY));

	// Log json
	Serial.printf("Processed Json %u, %u\n", static_cast<unsigned>(freq_hz), static_cast<unsigned>(duty));

	// Update frequency
	ledcChangeFrequency(pins::LED, freq_hz, PWM_RESOLUTION);

	// Update PWM
	// The LED is active low, so invert the PWM duty
	ledcWrite(pins::LED, MAX_DUTY - duty);
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
	m_json["buttonPresses"]	= press_count;
	m_json["buttonPressTime"] = press_duration;
}

String Json::to_string() const
{
	String json_string;

	(void)serializeJson(m_json, json_string);

	return json_string;
}