#include <Arduino.h>

#define BEAM_PIN 21
#define US 1'000'000

volatile uint32_t currTime		  = 0;
volatile uint32_t prevTime		  = 0;
volatile uint32_t totalTime		  = 0;
volatile bool	  timeChanged	  = false;
volatile uint32_t boxCount		  = 0;
volatile bool	  boxCountChanged = false;

void IRAM_ATTR onRising()
{
	// Get time in micros now
	uint32_t now = micros();

	// Last total is now the previous total
	prevTime = totalTime;

	// Calculate the total and record the current time
	totalTime	= now - currTime;
	currTime	= now;
	timeChanged = totalTime != prevTime;

	// Change the box count
	boxCount++;
	boxCountChanged = true;
}

void setup()
{
	// Start serial monitor
	Serial.begin(9600);

	// Setup input pin and attach interrupt
	pinMode(BEAM_PIN, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(BEAM_PIN), onRising, RISING);
}

void loop()
{
	// If the box count has changed
	if (boxCountChanged)
	{
		// Print the box count
		Serial.printf("Products produced: %d\n", boxCount);
		boxCountChanged = false;
	}

	// If the time has changed
	if (timeChanged)
	{
		// Calculate the frequency and print
		uint32_t boxesPerSecond = 1.0f / ((float)totalTime / (float)US);
		Serial.printf("Product output: %d per second\n", boxesPerSecond);
		timeChanged = false;
	}
}
