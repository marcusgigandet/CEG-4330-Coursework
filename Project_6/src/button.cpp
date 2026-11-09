#include "button.hpp"
#include "board.hpp"
#include "freertos/idf_additions.h"
#include <Arduino.h>

void Button::button_task(void* parameter)
{
	auto* button = static_cast<Button*>(parameter);
	uint8_t last_debounced_state;
	uint8_t current_debounced_state;

	while (true)
	{
		// Get current state
		last_debounced_state = button->m_debounced_state;

		// Update button
		button->process();

		current_debounced_state = button->m_debounced_state;

		// Check if button is just pressed
		if ((last_debounced_state != current_debounced_state) && (last_debounced_state == HIGH))
		{
			// Update json entries
			button->m_json.update_json(button->press_count(), button->press_duration());

			// Log to serial
			Serial.println(button->m_json.to_string());

			// Log to Python if connected
		}

		last_debounced_state = current_debounced_state;

		vTaskDelay(1);
	}
}

void Button::process()
{
	const uint8_t RAW_BUTTON_STATE{static_cast<uint8_t>(digitalRead(m_pin))};
	const uint32_t CURRENT_TIME{millis()};

	// The raw input changed, start the debounce period
	if (RAW_BUTTON_STATE != m_raw_state)
	{
		m_raw_state			  = RAW_BUTTON_STATE;
		m_debounce_start_time = CURRENT_TIME;
	}

	// Check for valid duration to prevent erroneous button presses
	// Also check that the state has changed to prevent recording multiple presses
	if (((CURRENT_TIME - m_debounce_start_time) >= m_debounce_delay_ms) &&
		(m_raw_state != m_debounced_state))
	{
		// Update the state of the debounced press
		m_debounced_state = m_raw_state;

		// Falling edge: button pressed
		if (m_raw_state == LOW)
		{
			m_press_start_time = CURRENT_TIME;
			++m_press_count;
		}
	}

	// Update press duration
	if (m_debounced_state == LOW)
	{
		m_press_duration = CURRENT_TIME - m_press_start_time;
	}
	else
	{
		m_press_duration = 0;
	}
}
