#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include "esp_sleep.h"
#include <Arduino.h>

// Pins
constexpr uint8_t LED_PIN{2};
constexpr uint8_t BUTTON_PIN{23};

// Global variables
constexpr uint32_t DEBOUNCE_WAIT_MS{50};
constexpr uint32_t SLEEP_WAIT_MS{5000};
uint8_t			   buttonState{LOW};

// ISR variables
volatile bool	  changePending{};
volatile uint32_t lastDebounceChange{};

void IRAM_ATTR debounceISR()
{
    // Update pending state
	lastDebounceChange = millis();
	changePending	   = true;
}

void setup()
{
	// LED
	pinMode(LED_PIN, OUTPUT);

	// Button
	pinMode(BUTTON_PIN, INPUT_PULLDOWN);
	attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), debounceISR, CHANGE);

	// Sleep
	esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(BUTTON_PIN), 1);
}

void loop()
{
	uint32_t currentTime{millis()};

	if (!changePending)
	{
		// Check if no button has been pressed for a long duration
		if ((currentTime - lastDebounceChange) > SLEEP_WAIT_MS)
		{
			esp_light_sleep_start();
		}
		return;
	}

	// Check for debounce period to pass
	if (currentTime - lastDebounceChange > DEBOUNCE_WAIT_MS)
	{
		buttonState	  = digitalRead(BUTTON_PIN);
		changePending = false;

		digitalWrite(LED_PIN, buttonState);
	}
}
