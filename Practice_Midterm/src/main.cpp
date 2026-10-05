// Problem:
// Write an ESP32 program using the Arduino libraries to do the following:

// Use a GPIO pin and interrupt to measure the speed and total number of products passing over a
// conveyor belt. Assume that there is a laser sensor that the products intersect with as they move
// along the conveyor belt. The laser is active high, outputting a high voltage when a product
// intersects the laser beam.


// You must print two quantities to the serial monitor, only if they change:

// The total number of products that have passed by the laser
// The frequency that the products are moving on the conveyor belt

#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include <Arduino.h>
#include <cstdint>

struct BeltState
{
	uint32_t productCount{};
	uint32_t totalTime{};
	uint32_t lastTime{};
	bool	 hasNewProduct{};
	bool	 hasNewFrequency{};
};

// Pins
constexpr uint32_t BUTTON_PIN{23};

// Global variables
volatile BeltState state{};

void IRAM_ATTR risingEdgeISR()
{
	uint32_t currentTime{micros()};
	uint32_t totalTime{currentTime - state.lastTime};

	// Check if a valid frequency exists
	// Or if the frequency value changed
	if ((state.lastTime != 0) || (totalTime != state.totalTime))
	{
		// Update frquency state
		state.totalTime		  = totalTime;
		state.hasNewFrequency = true;
	}

	// Update state
	state.lastTime = currentTime;
	state.productCount += 1;
	state.hasNewProduct = true;
}

void setup()
{
	// Serial
	Serial.begin(9600);

	// Button
	pinMode(BUTTON_PIN, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), risingEdgeISR, RISING);
}

void loop()
{
	BeltState localState;

	noInterrupts();
	{
		// Copy vars
		localState.totalTime	   = state.totalTime;
		localState.productCount	   = state.productCount;
		localState.hasNewProduct   = state.hasNewProduct;
		localState.hasNewFrequency = state.hasNewFrequency;

		// Clear the notification flags in the shared state.
		state.hasNewProduct	  = false;
		state.hasNewFrequency = false;
	}
	interrupts();

	if (localState.hasNewFrequency)
	{
		Serial.printf("Frequency: %.2f\n", 1'000'000.0f / (float)localState.totalTime);
	}

	if (localState.hasNewProduct)
	{
		Serial.printf("Product count: %u\n", localState.productCount);
	}
}
