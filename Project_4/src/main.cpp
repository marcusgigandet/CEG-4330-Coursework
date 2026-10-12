#include "common.hpp"
#include "esp32-hal-gpio.h"
#include "esp32-hal-ledc.h"
#include "esp32-hal.h"
#include "esp_attr.h"
#include <Arduino.h>

constexpr uint8_t RESOLUTION = 12;
constexpr uint8_t PWM_BITS	 = 8; // duty range 0-255
constexpr uint8_t PWM_PIN	 = 2; // onboard LED
constexpr uint8_t IN_PIN	 = 4; // connect to PWM_PIN with a jumper wire

void IRAM_ATTR pwmISR()
{
	static uint32_t lastTimeUS	  = 0;
	uint32_t		currentTimeUS = micros();

	if (HIGH == digitalRead(IN_PIN))
	{
		highSumUS = highSumUS + currentTimeUS - lastTimeUS;
	}
	else
	{
		lowSumUS = lowSumUS + currentTimeUS - lastTimeUS;
		edgeCount++; // Increment every cycle
	}

	lastTimeUS = currentTimeUS;
}

void setup()
{
	// Serial
	Serial.begin(BAUDRATE);

	// PWM_PIN
	ledcAttach(PWM_PIN, freq, PWM_BITS);
	ledcWrite(PWM_PIN, duty);

	// IN_PIN
	pinMode(IN_PIN, INPUT);
	attachInterrupt(IN_PIN, pwmISR, CHANGE);
}

void changeDutyCycle(long value)
{
	// Ignore invalid numbers
	if (value > 255)
	{
		return;
	}

	Serial.printf("Set duty: %ld", value);
	ledcWrite(PWM_PIN, value);
}

void changeFrequency(long value)
{
	// Ignore invalid numbers
	if (value < 100 || value > 10000)
	{
		return;
	}

	Serial.printf("Set frequency: %ld", value);
	ledcChangeFrequency(PWM_PIN, value, 12);
}

void getSerialInput()
{
	if (!Serial.available())
	{
		return;
	}

	String str		 = Serial.readString();
	long   value	 = str.substring(1).toInt();
	char   character = str.charAt(0);

	switch (character)
	{
	case 'd':
		changeDutyCycle(value);
		break;
	case 'f':
		changeFrequency(value);
		break;
	default:
		break;
	}

	str.charAt(0);
}

void loop()
{
	static uint32_t lastTime	= 0;
	uint32_t		currentTime = millis();

	// Display PWM metrics every second
	if (currentTime - lastTime > 1000)
	{
		displayIncomingPWM();
		lastTime = currentTime;
	}

	// Get the input from the serial monitor for changing frequency and duty cycle
	getSerialInput();

	// Display the incoming PWM signal's duty cycle and frequency
	displayIncomingPWM();
}
