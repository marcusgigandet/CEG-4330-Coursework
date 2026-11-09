#include "esp_camera.h"
#include "freertos/idf_additions.h"
#include <ArduinoJson.h>
#include <WiFi.h>

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

// Camera Settings
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

constexpr uint32_t STACK_SIZE = 4096;

WiFiServer g_camera_server;
WiFiServer g_json_server;
WiFiClient g_camera_client;
WiFiClient g_json_client;

JsonDocument g_json;

// RTOS: mutex that locks the json document and g_json_client, and the task handles
SemaphoreHandle_t g_json_mutex;
TaskHandle_t	  g_send_image_to_client_task_handle;
TaskHandle_t	  g_handle_json_from_client_task_handle;
TaskHandle_t	  g_button_task_handle;

void send_image_to_client_task(void* parameter);
void connect_to_wifi(const char* ssid, const char* password);
void start_camera_server(int port);

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

void start_camera_server(int port)
{
	// If we're not connected to WiFi, do nothing
	if (WiFi.status() != WL_CONNECTED)
	{
		return;
	}

	// Setup the camera settings
	camera_config_t config = {};
	config.ledc_channel	   = LEDC_CHANNEL_0;
	config.ledc_timer	   = LEDC_TIMER_0;
	config.pin_d0		   = Y2_GPIO_NUM;
	config.pin_d1		   = Y3_GPIO_NUM;
	config.pin_d2		   = Y4_GPIO_NUM;
	config.pin_d3		   = Y5_GPIO_NUM;
	config.pin_d4		   = Y6_GPIO_NUM;
	config.pin_d5		   = Y7_GPIO_NUM;
	config.pin_d6		   = Y8_GPIO_NUM;
	config.pin_d7		   = Y9_GPIO_NUM;
	config.pin_xclk		   = XCLK_GPIO_NUM;
	config.pin_pclk		   = PCLK_GPIO_NUM;
	config.pin_vsync	   = VSYNC_GPIO_NUM;
	config.pin_href		   = HREF_GPIO_NUM;
	config.pin_sccb_sda	   = SIOD_GPIO_NUM;
	config.pin_sccb_scl	   = SIOC_GPIO_NUM;
	config.pin_pwdn		   = PWDN_GPIO_NUM;
	config.pin_reset	   = RESET_GPIO_NUM;
	config.xclk_freq_hz	   = 20'000'000;
	config.pixel_format	   = PIXFORMAT_JPEG;
	// Adjust the size and quality to your liking
	// NOTE: Higher quality and size may result in slow transmissions
	config.frame_size =
		FRAMESIZE_QVGA; // Size of the output image: FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
	config.jpeg_quality = 10; // Quality of JPEG output. 0-63 lower means higher quality
	config.fb_count		= 1;

	// Initialize the camera with the settings
	esp_err_t err = esp_camera_init(&config);
	if (err != ESP_OK)
	{
		Serial.printf("Camera init failed with error 0x%x\n", err);
		return;
	}

	// Start the server and wait for clients
	g_camera_server = WiFiServer(port);
	g_camera_server.begin();
	Serial.println("Camera server started on port " + String(port));
}

void start_json_server(int port)
{
  	// Return early on no WiFi connection
	if (WiFi.status() != WL_CONNECTED)
	{
		return;
	}

	// Start server
	g_json_server = WiFiServer(port);
	g_json_server.begin();
}

void update_json()
{
	// g_json["buttonPresses"]	= buttonPresses;
	// g_json["buttonPressTime"] = buttonPressTime;
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
	start_camera_server(CAMERA_PORT);

	// Start the JSON server
	start_json_server(JSON_PORT);

	// Initialize the JSON (no tasks running yet, so no lock needed)
	update_json();

	// Create task for sending image
	xTaskCreate(
		send_image_to_client_task,
		"send_image_to_client",
		STACK_SIZE,
		nullptr,
		1,
		&g_send_image_to_client_task_handle);

	Serial.println("Setup complete!");
}

bool send_json_to_client(const WiFiClient& client)
{
	return false;
}

bool send_image_to_client(WiFiClient& client)
{
	// Get a pointer to the frame buffer of the camera
	camera_fb_t* frame_buf = esp_camera_fb_get();

	// If the frame buffer is null (we did not get an image!)
	if (nullptr == frame_buf)
	{
		Serial.println("Capture failed");
		return true;
	}

	// Send the frame buffer (4-byte length, then the JPEG)
	client.write(frame_buf->buf, frame_buf->len);

	// Tell the camera that it can reuse this frame buffer now that we've sent it off
	esp_camera_fb_return(frame_buf);

	return false;
}

void handle_json_from_client(const WiFiClient& client)
{
}

// Task: accept a camera client and stream frames to it
void send_image_to_client_task(void* parameter)
{
	while (true)
	{
		if (0 == g_camera_client.connected())
		{
			g_camera_client = g_camera_server.accept();
		}
		if (1 == g_camera_client.connected())
		{
			send_image_to_client(g_camera_client);
		}
		vTaskDelay(1);
	}
}

// Task: accept a JSON client, send it the current state, handle what it sends
void handle_json_from_client_task(void* parameter)
{
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
