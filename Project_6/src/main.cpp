#include "board.hpp"
#include "button.hpp"
#include "camera.hpp"
#include "json.hpp"
#include "secrets.hpp"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <freertos/idf_additions.h>

constexpr uint32_t CAMERA_PORT = 8000;
constexpr uint32_t JSON_PORT   = 8001;

/// Timeout is needed to continue to camera processing quickly
constexpr uint32_t SERIAL_TIMEOUT_MS = 10;
constexpr uint32_t BAUD_RATE		 = 9'600;
constexpr uint32_t DEBOUNCE_MS		 = 30;

constexpr uint32_t STACK_SIZE		  = 4096;
constexpr uint32_t RETRY_WIFI_CONN_MS = 5'000;


Button* g_button;
Camera* g_camera;

TaskHandle_t g_send_image_to_client_task_handle;
TaskHandle_t g_handle_json_from_client_task_handle;
TaskHandle_t g_button_task_handle;

void connect_to_wifi(const char* ssid, const char* password);
void create_tasks();

void connect_to_wifi(const char* ssid, const char* password)
{
	uint32_t start_time_ms	 = millis();
	uint32_t current_time_ms = millis();

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		if (current_time_ms - start_time_ms > RETRY_WIFI_CONN_MS)
		{
			// Ensure WiFi is disconnected before attempting to reconnect
			WiFi.disconnect();
			Serial.println("Failed to establish WiFi connection, retrying...");
			WiFi.begin(ssid, password);
			start_time_ms = millis();
		}

		// Update state
		current_time_ms = millis();
	}
}

void create_tasks()
{
	xTaskCreate(
		Camera::send_image_to_client_task,
		"send_image_to_client",
		STACK_SIZE,
		g_camera,
		1,
		&g_send_image_to_client_task_handle);

	xTaskCreate(
		Json::handle_json_from_client_task,
		"handle_json_from_client",
		STACK_SIZE,
		&g_button->json(),
		1,
		&g_handle_json_from_client_task_handle);

	xTaskCreate(
		Button::button_task,
		"process_button",
		STACK_SIZE,
		g_button,
		1,
		&g_button_task_handle);
}

void setup()
{
	// Initialize serial communications
	Serial.begin(BAUD_RATE);
	Serial.setTimeout(SERIAL_TIMEOUT_MS);

	// Set LED to off
	// Use active low for LED for this MCU
	// This means updating duty cycle to be reversed
	pinMode(pins::LED, OUTPUT);
	digitalWrite(pins::LED, HIGH);
	ledcAttachChannel(pins::LED, MAX_FREQUENCY_HZ, PWM_RESOLUTION, PWM_CHANNEL);
	ledcWrite(pins::LED, MAX_DUTY);

	// Connect to the SSID with PASSWORD
	connect_to_wifi(SSID, PASSWORD);

	Json json{
		xSemaphoreCreateMutex(),
		JSON_PORT,
	};

	g_camera = new Camera{CAMERA_PORT};
	g_button = new Button{
		ButtonDesc{
			.pin			   = pins::BUTTON,
			.debounce_delay_ms = DEBOUNCE_MS,
		},
		json,
	};

	create_tasks();

	Serial.println("Setup complete!");
}

void loop()
{
	// Delete Arduino loop task
	vTaskDelete(nullptr);
}
