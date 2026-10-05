// Problem:
// Write an ESP32 program using the Arduino libraries to do the following:
// - Using a button connected to a GPIO pin, record button the timings of button presses
//   and releases using debouncing. Then, if the button is not pressed for at least 5 seconds,
//   output the timing pattern of the button presses and releases on the onboard LED.
// - For example, if the button is held down for 3 seconds, released for 1 second,
//   held for 2 seconds, then released for 5 seconds, afterwards the LED should
//   turn on for 3 seconds, off for 1 second, and back on for 2 seconds, then off for 5 seconds,
//   then repeat this pattern forever until the user presses the button again.

#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include <Arduino.h>
#include <cstdint>

struct ButtonState
{
	uint8_t	 debouncedState{};	 // Debounced state
	uint8_t	 lastRawState{};	 // Previous raw reading
	uint32_t lastDebounceTime{}; // Last time the input changed
	uint32_t stateStartTime{};	 // Last time the state started
};

struct SequenceState
{
	uint32_t sequences[UINT8_MAX + 1]{};
	uint8_t	 head{};
	uint8_t	 tail{};
	bool	 sequenceStarted{};
	bool	 recording{true};
};

// Pins
constexpr uint32_t LED_PIN{2};
constexpr uint32_t BUTTON_PIN{23};

// Global variables
constexpr uint32_t DEBOUNCE_DELAY_MS{50};
constexpr uint32_t WAIT_MS{5'000};
ButtonState		   buttonState{};
SequenceState	   sequenceState{};

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
	if (((currentTime - buttonState.lastDebounceTime) > DEBOUNCE_DELAY_MS) &&
		(currentReading != buttonState.debouncedState))
	{
		if (currentReading == HIGH)
		{
			// A press during playback starts a new recording
			if (!sequenceState.recording)
			{
				sequenceState.recording = true;
				sequenceState.head		= 0;
				sequenceState.tail		= 0;
				digitalWrite(LED_PIN, LOW);
			}
			// Discards the first LOW duration (head == 0), records all other LOWs
			else if (
				buttonState.debouncedState == LOW && sequenceState.head > 0 &&
				sequenceState.head < 256)
			{
				sequenceState.sequences[sequenceState.head] =
					currentTime - buttonState.stateStartTime;
				sequenceState.head = sequenceState.head + 1;
			}
		}
		else if (
			sequenceState.recording && buttonState.debouncedState == HIGH &&
			sequenceState.head < 256)
		{
			// The release ends a press segment
			sequenceState.sequences[sequenceState.head] = currentTime - buttonState.stateStartTime;
			sequenceState.head							= sequenceState.head + 1;
		}

		// Update debounce logic
		buttonState.debouncedState = currentReading;
		buttonState.stateStartTime = buttonState.lastDebounceTime;
	}

	return buttonState.debouncedState == HIGH;
}

void setup()
{
	// Serial
	Serial.begin(9600);

	// LED
	pinMode(LED_PIN, OUTPUT);

	// Button
	pinMode(BUTTON_PIN, INPUT_PULLDOWN);
}

void loop()
{
	static uint32_t sequenceStartTime{};
    bool isPressed = isButtonPressed();

	if (sequenceState.recording)
	{
		// Check if the recording ended
		if (!isPressed && (millis() - buttonState.stateStartTime) > WAIT_MS)
		{
			// Set the final index
			sequenceState.sequences[sequenceState.head] = WAIT_MS;
			sequenceState.head++;

			// Begin playing the sequences
			sequenceState.recording		  = false;
			sequenceState.sequenceStarted = false;
		}
	}
	else
	{
		// Play the sequence
		if (!sequenceState.recording && sequenceState.head > 0)
		{
			if (!sequenceState.sequenceStarted)
			{
				sequenceState.sequenceStarted = true;
				sequenceStartTime			  = millis();

				// Write HIGH for even, LOW for odd
				digitalWrite(LED_PIN, 0 == sequenceState.tail % 2);
			}

			// Check if the current sequence has finished
			if (sequenceState.sequenceStarted &&
				(millis() - sequenceStartTime) > sequenceState.sequences[sequenceState.tail])
			{
				// Reset state
				sequenceState.sequenceStarted = false;

				// Increment to the next index
				sequenceState.tail = (sequenceState.tail + 1) % sequenceState.head;
			}
		}
	}
}
