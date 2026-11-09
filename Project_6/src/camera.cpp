#include "camera.hpp"
#include <esp_camera.h>
#include <freertos/idf_additions.h>
#include <ArduinoJson.h>
#include <WiFi.h>

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

void Camera::send_image_to_client_task(void* parameter)
{
	auto* camera = static_cast<Camera*>(parameter);

	while (true)
	{
		if (0 == camera->m_camera_client.connected())
		{
			camera->m_camera_client = camera->m_camera_server.accept();
		}

		if (1 == camera->m_camera_client.connected())
		{
			camera->send_image_to_client(camera->m_camera_client);
		}
		vTaskDelay(1);
	}
}

void Camera::start_server(int port)
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
	m_camera_server = WiFiServer(port);
	m_camera_server.begin();
	Serial.println("Camera server started on port " + String(port));
}

bool Camera::send_image_to_client(WiFiClient& client)
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
