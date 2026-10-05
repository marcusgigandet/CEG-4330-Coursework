#include <Arduino.h>

// Pins
constexpr uint32_t BUTTON_PIN{23};

// Global variables
constexpr uint32_t DEBOUNCE_DELAY_MS{50};

struct ButtonState
{
	uint8_t	 debouncedState;   // Debounced state
	uint8_t	 lastRawState;	   // Previous raw reading
	uint32_t lastDebounceTime; // Last time the input changed
};

bool isButtonPressed()
{
	static ButtonState buttonState{
		LOW,
		LOW,
		0,
	};
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
		buttonState.debouncedState = currentReading;
	}

	return buttonState.debouncedState == HIGH;
}

void setup()
{
	// Serial
	Serial.begin(9600);

	// Button
	pinMode(BUTTON_PIN, INPUT_PULLDOWN);
}

void loop()
{
	static bool lastButtonState{LOW};
	bool		currentButtonState{isButtonPressed()};

	// Detect rising edge
	if (currentButtonState == HIGH && lastButtonState == LOW)
	{
		Serial.println("Pressed");
	}

	lastButtonState = currentButtonState;
}
