#include "board.hpp"
#include "button.hpp"
#include "camera.hpp"
#include "json.hpp"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <freertos/idf_additions.h>

// WiFi Settings
const char* SSID = "your-ssid-here";
const char* PASSWORD = "your-password-here";

constexpr uint32_t CAMERA_PORT = 8000;
constexpr uint32_t JSON_PORT   = 8001;

constexpr uint32_t BAUD_RATE = 9'600;

constexpr uint32_t DEBOUNCE_MS = 30;

/// Channel to use for PWM.
/// @note LEDC channel 0 is used by the camera clock, so don't use it for the LED
constexpr uint8_t PWM_CHANNEL = 4;

constexpr uint32_t STACK_SIZE = 4096;


Button g_button{ButtonDesc{
	.pin			   = pins::BUTTON,
	.debounce_delay_ms = DEBOUNCE_MS,
}};
Camera g_camera;
Json g_json;

/// RTOS mutex that locks the json document and g_json_client, and the task handles
SemaphoreHandle_t g_json_mutex;

TaskHandle_t g_send_image_to_client_task_handle;
TaskHandle_t g_handle_json_from_client_task_handle;
TaskHandle_t g_button_task_handle;

void connect_to_wifi(const char* ssid, const char* password);
void create_tasks();

void connect_to_wifi(const char* ssid, const char* password)
{
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		WiFi.disconnect();
		Serial.println("Failed to establish WiFi connection, retrying...");
		WiFi.begin(ssid, password);
		vTaskDelay(5'000);
	}
}

void create_tasks()
{
	xTaskCreate(
		Camera::send_image_to_client_task,
		"send_image_to_client",
		STACK_SIZE,
		&g_camera,
		1,
		&g_send_image_to_client_task_handle);

	xTaskCreate(
		Json::handle_json_from_client_task,
		"handle_json_from_client",
		STACK_SIZE,
		&g_json,
		1,
		&g_handle_json_from_client_task_handle);

	xTaskCreate(
		Button::button_task,
		"process_button",
		STACK_SIZE,
		&g_button,
		1,
		&g_button_task_handle);
}

void setup()
{
	// Start the serial monitor
	Serial.begin(BAUD_RATE);
	Serial.setTimeout(10); // Timeout is needed to continue to camera processing quickly

	// Set LED to off
	pinMode(pins::LED, OUTPUT);
	digitalWrite(pins::LED, LOW);

	// Connect to the SSID with PASSWORD
	connect_to_wifi(SSID, PASSWORD);

	g_camera = Camera(CAMERA_PORT);
	g_json = Json(JSON_PORT);
	g_json.update_json();

	create_tasks();

	Serial.println("Setup complete!");
}

void loop()
{
	// Delete Arduino loop task
	vTaskDelete(nullptr);
}
