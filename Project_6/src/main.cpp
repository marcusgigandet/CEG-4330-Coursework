#include "button.hpp"
#include "camera.hpp"
#include "json.hpp"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <esp_camera.h>
#include <freertos/idf_additions.h>

// WiFi Settings
#define SSID "your-ssid-here"
#define PASSWORD "your-password-here"
#define CAMERA_PORT 8000
#define JSON_PORT 8001

// Button and LED Settings
#define BUTTON_PIN 15 // Physical pin 23, button to ground
#define LED_PIN 2	  // Onboard LED
#define DEBOUNCE_MS 30
#define PWM_CHANNEL 4 // LEDC channel 0 is used by the camera clock, so don't use it for the LED

constexpr uint32_t STACK_SIZE = 4096;

Camera g_camera;
Json g_json;

// RTOS: mutex that locks the json document and g_json_client, and the task handles
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
}

void setup()
{
	// Start the serial monitor
	Serial.begin(9600);
	Serial.setTimeout(10); // Timeout is needed to continue to camera processing quickly

	// Button with internal pull up, LED off
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);

	// Connect to the SSID with PASSWORD
	connect_to_wifi(SSID, PASSWORD);

	// Start the camera server
	g_camera = Camera(CAMERA_PORT);

	// Start the JSON server
	g_json = Json(JSON_PORT);

	// Initialize the JSON (no tasks running yet, so no lock needed)
	g_json.update_json();

  create_tasks();

	Serial.println("Setup complete!");
}

// Task: on a debounced button press, update the JSON, print it and send it
void button_task(void* parameter)
{
}

void loop()
{
	// Delete Arduino loop task
	vTaskDelete(nullptr);
}
