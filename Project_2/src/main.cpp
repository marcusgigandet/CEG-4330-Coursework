/*
 * Copyright (C) 2026 Marcus Gigandet
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include <cmath>
#include <cstddef>

namespace
{
	constexpr uint8_t ROWS{4};
	constexpr uint8_t COLS{4};

	namespace Pins
	{
		constexpr uint8_t LED{2};
		constexpr uint8_t BUTTON{12};
		constexpr uint8_t SPEAKER{18};

		constexpr uint8_t ROW[ROWS]{13, 14, 27, 26};
		constexpr uint8_t COL[COLS]{25, 33, 32, 4};
	} // namespace Pins

	constexpr uint32_t BAUD_RATE{9600};

	constexpr uint16_t DEBOUNCE_DELAY_MS{10};
	constexpr uint16_t TONE_DURATION_MS{100};

	uint32_t octave{};

	constexpr char keypad[ROWS][COLS]{
		{'1', '2', '3', 'A'},
		{'4', '5', '6', 'B'},
		{'7', '8', '9', 'C'},
		{'*', '0', '#', 'D'},
	};

	// clang-format off
	constexpr uint8_t N_TABLE[ROWS][COLS]{
		40, 41, 42, 0,
		43, 44, 45, 0,
		46, 47, 48, 0,
		49, 50, 51, 0,
	};
	// clang-format on

	struct ButtonData
	{
		/// Raw, current state of the button
		uint8_t rawState{LOW};

		/// Debounced state of the button press
		uint8_t debouncedState{LOW};

		/// Start time of the debouncing
		uint32_t debounceStartTime{};

		/// Start time of the actual button press
		uint32_t pressStartTime{};

		/// Count of intentional button presses
		uint32_t pressCount{};
	};

	ButtonData buttonData{};
} // namespace

void setup()
{
	Serial.begin(BAUD_RATE);
	Serial.println("Starting program...");

	// Set LED to on
	pinMode(Pins::LED, OUTPUT);
	digitalWrite(Pins::LED, HIGH);

	// Configure speaker to output
	pinMode(Pins::SPEAKER, OUTPUT);

	// Configure standalone button pin
	pinMode(Pins::BUTTON, INPUT_PULLDOWN);

	// Configure keypad matrix inputs
	for (uint32_t row{}; row < ROWS; ++row)
	{
		pinMode(Pins::ROW[row], INPUT_PULLUP);
	}

	// Configure keypad matrix outputs
	for (uint32_t col{}; col < COLS; ++col)
	{
		pinMode(Pins::COL[col], OUTPUT);
		digitalWrite(Pins::COL[col], LOW);
	}
}

/**
 * @brief Handles the button presses for the standalone button.
 *
 * This handles any debouncing before confirming the button press.
 */
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

		// Rising edge: button pressed.
		if (rawButtonState == HIGH)
		{
			buttonData.pressStartTime = currentTime;
			++buttonData.pressCount;
			++octave;
		}

		// Falling edge: button released.
		else
		{
			// Log the duration of time that the button was pressed starting from the initial
			// press
			Serial.printf(
				"Press count: %u - Press time: %f s\n",
				buttonData.pressCount,
				(currentTime - buttonData.pressStartTime) / 1000.0);
		}
	}
}

/**
 * @brief Processes the keypad matrix and returns the current button press.
 *
 * @return Mapped key value.
 */
uint8_t getKeypadPress()
{
	for (size_t c{}; c < COLS; ++c)
	{
		// Select the current column for reading
		digitalWrite(Pins::COL[c], LOW);

		for (size_t r{}; r < ROWS; ++r)
		{
			// Check if the current element is pressed
			if (LOW == digitalRead(Pins::ROW[r]))
			{
				// Reset pin state
				pinMode(Pins::COL[c], HIGH);

				// Return the mapped value
				// Also calculate current octave * 12
				return N_TABLE[r][c] + octave * 12;
			}
		}

		// Reset pin state
		digitalWrite(Pins::COL[c], HIGH);
	}

	// Base case where no keys are pressed
	return 0;
}

/**
 * @brief Takes in a value, n, and plays a tone using the key in the calculated frequency.
 * @param n Value to use in the calculation.
 *
 * @note Does nothing for a frequency of 0.
 */
void handleTone(uint8_t n)
{
	const uint32_t frequency{440 * static_cast<uint32_t>(pow(2, ((n - 49)) / 12.0))};

	// Ignore invalid frequencies
	if (0 < frequency)
	{
		tone(Pins::SPEAKER, frequency, TONE_DURATION_MS);
	}
}

void loop()
{
	handleButtonPress();

	uint8_t n{getKeypadPress()};
	handleTone(n);
}
