#include <Arduino.h>

#define BUTTON_PIN 23
#define DEBOUNCE_DELAY 50 // ms

struct ButtonState
{
	uint8_t	 debouncedState;   // Debounced state
	uint8_t	 lastRawState;	   // Previous raw reading
	uint32_t lastDebounceTime; // Last time the input changed
};

bool isButtonPressed()
{
	static ButtonState buttonState = {LOW, LOW, 0};
	uint32_t		   currentTime = millis();

	// Read current button state
	uint8_t currentReading = (REG_READ(GPIO_IN_REG) >> BUTTON_PIN) & 1;

	// Check if raw state has changed
	if (currentReading != buttonState.lastRawState)
	{
		buttonState.lastDebounceTime = currentTime;
		buttonState.lastRawState	 = currentReading;
	}

	// If enough time has passed since last change, update debounced state
	if (currentTime - buttonState.lastDebounceTime > DEBOUNCE_DELAY)
	{
		buttonState.debouncedState = currentReading;
	}

	return buttonState.debouncedState == HIGH;
}

void setup()
{
	Serial.begin(9600);

	pinMode(BUTTON_PIN, INPUT_PULLDOWN);
}

void loop()
{
	static bool lastButtonState	   = LOW;
	bool		currentButtonState = isButtonPressed();

	// Detect rising edge
	if (currentButtonState == HIGH && lastButtonState == LOW)
	{
		Serial.println("Pressed");
	}

	lastButtonState = currentButtonState;
}
