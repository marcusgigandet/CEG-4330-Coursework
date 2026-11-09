#pragma once

#include <Arduino.h>

struct ButtonDesc
{
	uint8_t pin;
	uint32_t debounce_delay_ms = 30;
};

class Button
{
	uint8_t m_pin;
	uint32_t m_debounce_delay_ms;

	/// Raw, current state of the button
	uint8_t m_raw_state;

	/// Debounced state of the button
	uint8_t m_debounced_state;

	/// Start time of the debouncing
	uint32_t m_debounce_start_time;

	/// Start time of the actual button press
	uint32_t m_press_start_time;

	/// Count of intentional button presses
	uint32_t m_press_count;

	/// Total duration of press
	uint32_t m_press_duration;

public:
	Button(const ButtonDesc& desc) :
		m_pin(desc.pin), m_debounce_delay_ms(desc.debounce_delay_ms)
	{
		// Configure pin
		pinMode(m_pin, INPUT_PULLUP);
	}

	static void button_task(void* parameter);

	/// @brief Handles the button presses for the standalone button.
	///
	/// This handles any debouncing before confirming the button press.
	/// This also logs the button press duration (press after debouncing to release) to serial comm.
	void handle_press();
};
