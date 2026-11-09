#include "button.hpp"
#include "board.hpp"
#include <Arduino.h>

constexpr uint32_t DEBOUNCE_DELAY_MS = 10;

void Button::handle_press()
{
	const uint8_t  RAW_BUTTON_STATE{static_cast<uint8_t>(digitalRead(Pins::BUTTON))};
	const uint32_t CURRENT_TIME{millis()};

	// The raw input changed, start the debounce period
	if (RAW_BUTTON_STATE != m_raw_state)
	{
		m_raw_state			 = RAW_BUTTON_STATE;
		m_debounce_start_time = CURRENT_TIME;
	}

	// Check for valid duration to prevent erroneous button presses
	// Also check that the state has changed to prevent recording multiple presses
	if (((CURRENT_TIME - m_debounce_start_time) >= DEBOUNCE_DELAY_MS) &&
		(m_raw_state != m_debounced_state))
	{
		// Update the state of the debounced press
		m_debounced_state = m_raw_state;

		// Rising edge: button pressed
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
}