#pragma once

#include "json.hpp"

#include <Arduino.h>

namespace
{
	#define DEFAULT_DEBOUNCE_DELAY 30
}

struct ButtonDesc
{
	uint8_t pin;
	uint32_t debounce_delay_ms = DEFAULT_DEBOUNCE_DELAY;
};

class Button
{
	Json m_json;
	uint8_t m_pin;
	uint32_t m_debounce_delay_ms;

	/// Raw, current state of the button
	uint8_t m_raw_state{};

	/// Debounced state of the button
	uint8_t m_debounced_state{};

	/// Start time of the debouncing
	uint32_t m_debounce_start_time{};

	/// Start time of the actual button press
	uint32_t m_press_start_time{};

	/// Count of intentional button presses
	uint32_t m_press_count{};

	/// Total duration of press
	uint32_t m_press_duration{};


	/// @brief Handles the button presses for the standalone button.
	///
	/// This handles any debouncing before confirming the button press.
	/// This also logs the button press duration (press after debouncing to release) to serial comm.
	void process();

public:
	Button(const ButtonDesc& desc, int json_port) :
		m_json(json_port), m_pin(desc.pin), m_debounce_delay_ms(desc.debounce_delay_ms)
	{
		// Configure pin
		pinMode(m_pin, INPUT_PULLUP);
		m_json.update_json(m_press_count, m_press_duration);
	}

	static void button_task(void* parameter);

	Json& json() { return m_json; }

	[[nodiscard]] uint32_t press_count() const { return m_press_count; }
	[[nodiscard]] uint32_t press_duration() const { return m_press_duration; }
};
