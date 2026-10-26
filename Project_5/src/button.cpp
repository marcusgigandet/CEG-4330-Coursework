#include "button.hpp"
#include "board.hpp"
#include <Arduino.h>

constexpr uint32_t DEBOUNCE_DELAY_MS = 10;

ButtonData buttonData{};

/// @brief Handles the button presses for the standalone button.
///
/// This handles any debouncing before confirming the button press.
/// This also logs the button press duration (press after debouncing to release) to serial comm.
void handleButtonPress()
{
	const uint8_t  rawButtonState{static_cast<uint8_t>(digitalRead(Pins::BUTTON))};
	const uint32_t currentTime{millis()};

	// The raw input changed, start the debounce period
	if (rawButtonState != buttonData.rawState)
	{
		buttonData.rawState			 = rawButtonState;
		buttonData.debounceStartTime = currentTime;
	}

	// Check for valid duration to prevent erroneous button presses
	// Also check that the state has changed to prevent recording multiple presses
	if (((currentTime - buttonData.debounceStartTime) >= DEBOUNCE_DELAY_MS) &&
		(buttonData.rawState != buttonData.debouncedState))
	{
		// Update the state of the debounced press
		buttonData.debouncedState = buttonData.rawState;

		// Rising edge: button pressed
		if (rawButtonState == HIGH)
		{
			buttonData.pressStartTime = currentTime;
			++buttonData.pressCount;
		}
	}

    buttonData.pressTime = currentTime - buttonData.pressStartTime;
}
