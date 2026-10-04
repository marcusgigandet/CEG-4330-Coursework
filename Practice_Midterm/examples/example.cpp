/// Minimal example program using interrupts to calculate belt speed.
///
/// The sensor was specified to be active-low when an object is detected.

#include <Arduino.h>

#define DISTANCE 10
#define EDGE_PIN 23

volatile float speed = 0.0f;
volatile uint32_t beltStartTime = 0;

void IRAM_ATTR edgeChangeISR()
{
	// Rising-edge
	if (digitalRead(EDGE_PIN) == HIGH)
	{
		// No object detected
		// Begin timing
		beltStartTime = micros();
	}

	// Falling-edge
	else
	{
		// Check that the belt was detected first
		if (beltStartTime != 0)
		{
			// Object detected
			// Stop timing
			// Calculate speed (mm/sec)
			speed = DISTANCE / ((micros() - beltStartTime) / 1'000'000.0f);
		}
	}
}

void setupEdgeISRs()
{
	pinMode(EDGE_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(EDGE_PIN), edgeChangeISR, CHANGE);
}

void setup()
{
	setupEdgeISRs();
}

void loop()
{
}
