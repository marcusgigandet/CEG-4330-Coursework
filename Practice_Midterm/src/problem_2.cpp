#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include <Arduino.h>

struct ButtonState
{
	uint8_t	 debouncedState{};	 // Debounced state
	uint8_t	 lastRawState{};	 // Previous raw reading
	uint32_t lastDebounceTime{}; // Last time the input changed
	uint32_t stateStartTime{};
};

struct BlinkState
{
	uint32_t indexStartTime{};
	uint32_t sequences[256]{};
	uint8_t	 head{};
	uint8_t	 tail{};
	bool	 recording{};
	bool	 playing{};
};

// Pins
constexpr uint32_t LED_PIN{2};
constexpr uint32_t BUTTON_PIN{23};

// Constants
constexpr uint32_t DEBOUNCE_DELAY_MS{1'000};
constexpr uint32_t TIMEOUT_MS{5'000};

// Global variables
ButtonState buttonState{};
BlinkState	blinkState{};

bool isButtonPressed()
{
	uint32_t currentTime{millis()};

	// Read current button state
	uint8_t currentReading = digitalRead(BUTTON_PIN);

	// Check if raw state has changed
	if (currentReading != buttonState.lastRawState)
	{
		buttonState.lastDebounceTime = currentTime;
		buttonState.lastRawState	 = currentReading;
	}

	// If enough time has passed since last change, update debounced state
	if (currentTime - buttonState.lastDebounceTime > DEBOUNCE_DELAY_MS)
	{
		// Reset blink state to defaults
		if (!blinkState.recording)
		{
			blinkState.head		 = 0;
			blinkState.tail		 = 0;
			blinkState.recording = true;
			blinkState.playing	 = false;
		}
		// Add duration to the sequence list
		else if (buttonState.debouncedState == HIGH || (blinkState.head > 0 && blinkState.head < 256 - 1))
		{
			blinkState.sequences[blinkState.head] = 1'000 * ((currentTime - buttonState.stateStartTime) / 1'000);
			blinkState.head++;
		}

		// Update the button state
		buttonState.debouncedState = currentReading;
		buttonState.stateStartTime = currentTime;
	}

	return buttonState.debouncedState == HIGH;
}

void setup()
{
	// Serial
	Serial.begin(9600);

	// Button
	pinMode(BUTTON_PIN, INPUT_PULLDOWN);

	// LED
	pinMode(LED_PIN, OUTPUT);
}

void loop()
{
	uint32_t currentTime = millis();
	bool	 isPressed	 = isButtonPressed();

	// Check if timeout has occurred
	if (!isPressed && currentTime - buttonState.stateStartTime > TIMEOUT_MS)
	{
		// Initialize blink state for playing the sequence
		blinkState.recording				  = false;
		blinkState.sequences[blinkState.head] = 5'000;
		blinkState.tail						  = 0;
		blinkState.head++;
	}

	// Process the blink state
	if (!blinkState.recording)
	{
		if (!blinkState.playing)
		{
			// Update the state
			blinkState.playing		  = true;
			blinkState.indexStartTime = currentTime;

			// Write the pin
			digitalWrite(LED_PIN, 0 == (blinkState.tail % 2));
		}

		// Check if the current index duration has exceeded
		if ((currentTime - blinkState.indexStartTime) > blinkState.sequences[blinkState.tail])
		{
			// Update the state
			blinkState.tail	   = (1 + blinkState.tail) % blinkState.head;
			blinkState.playing = false;
		}
	}
}
