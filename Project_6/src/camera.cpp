#include "esp_camera.h"
#include "freertos/idf_additions.h"
#include <ArduinoJson.h>
#include <WiFi.h>

bool send_image_to_client(const WiFiClient& client)
{
	// Get a pointer to the frame buffer of the camera
	camera_fb_t* frame_buf = esp_camera_fb_get();

	// If the frame buffer is null (we did not get an image!)
	if (!frame_buf)
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