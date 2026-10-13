#include "common.hpp"
#include <Arduino.h>

uint8_t  duty = 127;
uint16_t freq = 5000;

volatile uint32_t lowSumUS	= 0;
volatile uint32_t highSumUS = 0;
volatile uint32_t edgeCount = 0;

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

void displayIncomingPWM()
{
	// No edges captured since last display: no signal on IN_PIN
	if (edgeCount == 0)
	{
		Serial.println("No signal");
		return;
	}

	// One falling edge per cycle in a ~1 s window, so the cycle count is the frequency in Hz
	Serial.print("Freq: ");
	Serial.print(edgeCount);
	Serial.print(", Duty: ");
	Serial.println((float)highSumUS / (float)(lowSumUS + highSumUS));

	// Reset state
	edgeCount = 0;
	lowSumUS  = 0;
	highSumUS = 0;
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
}

void pwmISR()
{
	static uint32_t lastTimeUS	  = 0;
	uint32_t		currentTimeUS = micros();

	if (HIGH == digitalRead(IN_PIN))
	{
		lowSumUS = lowSumUS + currentTimeUS - lastTimeUS;
	}
	else
	{
		highSumUS = highSumUS + currentTimeUS - lastTimeUS;
		edgeCount = edgeCount + 1; // Increment every cycle
	}

	lastTimeUS = currentTimeUS;
}
