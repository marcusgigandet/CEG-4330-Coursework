// Problem:
// Write an ESP32 program using the Arduino libraries to do the following:
// - Using a button connected to a GPIO pin, record button the timings of button presses
//   and releases using debouncing. Then, if the button is not pressed for at least 5 seconds,
//   output the timing pattern of the button presses and releases on the onboard LED.
// - For example, if the button is held down for 3 seconds, released for 1 second,
//   held for 2 seconds, then released for 5 seconds, afterwards the LED should
//   turn on for 3 seconds, off for 1 second, and back on for 2 seconds, then off for 5 seconds,
//   then repeat this pattern forever until the user presses the button again.

#include <Arduino.h>

// Pins
constexpr uint32_t LED_PIN{2};
constexpr uint32_t BUTTON_PIN{23};

// Global variables
constexpr uint32_t WAIT_MS{5'000};
constexpr uint32_t DEBOUNCE_DELAY_MS{50};

/// Even indices are LED on, odd are LED off
uint32_t sequence[256]{};
uint16_t head{}; // Number of recorded segments
uint8_t	 tail{}; // Current segment being played

/// True while recording a new pattern, false while playing it back
bool recording{true};

struct ButtonState
{
	uint8_t	 debouncedState;   // Debounced state
	uint8_t	 lastRawState;	   // Previous raw reading
	uint32_t lastDebounceTime; // Last time the raw input changed
	uint32_t stateStartTime;   // Time the current debounced state began
};

ButtonState buttonState{
	LOW,
	LOW,
	0,
	0,
};

/// Debounce the button and record segment durations on state changes.
/// Returns true if the button is pressed.
bool isButtonPressed()
{
	uint32_t currentTime{static_cast<uint32_t>(millis())};

	// Read current button state
	uint8_t currentReading = digitalRead(BUTTON_PIN);

	// Check if raw state has changed
	if (currentReading != buttonState.lastRawState)
	{
		buttonState.lastDebounceTime = currentTime;
		buttonState.lastRawState	 = currentReading;
	}

	// Wait for steady state
	if (currentTime - buttonState.lastDebounceTime > DEBOUNCE_DELAY_MS
		&& currentReading != buttonState.debouncedState)
	{
		if (currentReading == HIGH)
		{
			// A press during playback starts a new recording
			if (!recording)
			{
				recording = true;
				head	   = 0;
				tail	   = 0;
				digitalWrite(LED_PIN, LOW);
			}
			// Discards the first LOW duration (head == 0), records all other LOWs
			else if (buttonState.debouncedState == LOW && head > 0 && head < 256)
			{
				sequence[head] = currentTime - buttonState.stateStartTime;
				head		   = head + 1;
			}
		}
		else if (recording && buttonState.debouncedState == HIGH && head < 256)
		{
			// The release ends a press segment
			sequence[head] = currentTime - buttonState.stateStartTime;
			head		   = head + 1;
		}

		buttonState.debouncedState = currentReading;
		buttonState.stateStartTime = currentTime;
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
	static bool		sequenceStarted{};
	static uint32_t sequenceStart{};

	bool pressed = isButtonPressed();

	// Wait until no activity for WAIT_MS time
	if (recording)
	{
		if (!pressed && head > 0
			&& millis() - buttonState.stateStartTime >= WAIT_MS)
		{
			// Close the pattern with the final release segment
			if (head < 256)
			{
				sequence[head] = WAIT_MS;
				head		   = head + 1;
			}

			// Start playback from the beginning of the pattern
			recording		= false;
			tail			= 0;
			sequenceStarted = false;
		}
	}
	// Playback: step through the recorded segments forever
	else
	{
		// Wait until the current segment finished its duration
		if (sequenceStarted && (millis() - sequenceStart) >= sequence[tail])
		{
			tail			= (tail + 1) % head;
			sequenceStarted = false;
		}

		// Start the next segment
		if (!sequenceStarted)
		{
			sequenceStart	= millis();
			sequenceStarted = true;

			// Even segments are LED on, odd are LED off
			if (tail % 2 == 0)
			{
				digitalWrite(LED_PIN, HIGH);
			}
			else
			{
				digitalWrite(LED_PIN, LOW);
			}
		}
	}
}
