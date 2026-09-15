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

#define LED_PIN 2
#define BUTTON_PIN 4
#define LOW_DURATION 6
#define HIGH_DURATION 2
#define DUTY_CYCLE HIGH_DURATION / (HIGH_DURATION + LOW_DURATION)

static void non_blocking_delay()
{
	// Get the current time count
	static uint32_t startTime = micros();
	uint32_t currentTime = micros();
	bool pinHigh = false;

	if (((currentTime - startTime) > LOW_DURATION) && (false == pinHigh)) {
		// Set to high
		digitalWrite(LED_PIN, HIGH);
		pinHigh = true;

		// Reset the start time
		startTime = micros();
	} else if (((currentTime - startTime) > HIGH_DURATION) && (true == pinHigh)) {
		// - Set to low
		digitalWrite(LED_PIN, LOW);
		pinHigh = false;

		// Reset the start time
		startTime = micros();
	}
}

void setup()
{
	pinMode(LED_PIN, OUTPUT);
}

void loop()
{
	non_blocking_delay();
}
