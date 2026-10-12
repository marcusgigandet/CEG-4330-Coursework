#include "common.hpp"
#include <Arduino.h>

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
	Serial.printf(
		"Freq: %ul, Duty: %0.2f",
		10000000 / edgeCount,
		(float)highSumUS / (float)(lowSumUS + highSumUS));

	// Reset state
	edgeCount = 0;
	lowSumUS  = 0;
	highSumUS = 0;
}