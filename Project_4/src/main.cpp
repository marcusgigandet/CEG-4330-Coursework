#include "common.hpp"
#include "esp32-hal-gpio.h"
#include "esp32-hal-ledc.h"
#include "esp32-hal.h"
#include "esp_attr.h"
#include <Arduino.h>

constexpr uint8_t PWM_BITS	 = 8; // duty range 0-255
constexpr uint8_t PWM_PIN	 = 2; // onboard LED

void setup()
{
	// Serial
	Serial.begin(BAUDRATE);

	// PWM_PIN
	ledcAttach(PWM_PIN, freq, PWM_BITS);
	ledcWrite(PWM_PIN, duty);

	// IN_PIN
	pinMode(IN_PIN, INPUT);
	attachInterrupt(digitalPinToInterrupt(IN_PIN), pwmISR, CHANGE);
}

void changeDutyCycle(long value)
{
	// Ignore invalid numbers
	if (value < 0 || value > 255)
	{
		return;
	}

	Serial.printf("Set duty: %ld\n", value);
	ledcWrite(PWM_PIN, value);
}

void changeFrequency(long value)
{
	// Ignore invalid numbers
	if (value < 100 || value > 10000)
	{
		return;
	}

	Serial.printf("Set frequency: %ld Hz\n", value);
	ledcChangeFrequency(PWM_PIN, value, PWM_BITS);
}
