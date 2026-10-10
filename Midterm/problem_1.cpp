// Using a button connected to a GPIO pin, record the number of button presses using debouncing.
// Then, if the button is not pressed for at least 5 seconds, blink the onboard LED at the 
// frequency of the number of button presses.
//
// For example, if the button is pressed 3 times and then released for 5 seconds,
// afterwards the LED should blink at 3Hz. Some time later, if the button is pressed
// 7 times and then released for 5 seconds, the LED should blink at 7Hz.


#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include "esp_attr.h"
#include <Arduino.h>

// Constants
constexpr uint32_t TIMEOUT_MS = 5'000;
constexpr uint32_t DEBOUNCE_MS = 50; // Small delay for debouncing

// Pins
constexpr uint8_t LED_PIN = 2;
constexpr uint8_t BUTTON_PIN = 23;

// Globals
// Button
uint32_t totalPressCount = 0;
uint32_t lastDebouncePressTimeMS = 0;

bool isButtonPressed()
{
    static uint32_t lastRawButtonPressTimeMS = 0;
    static bool lastRawButtonPressState = false; // True if pressed
    static bool currentDebouncedPressState = false; 
    uint32_t currentTimeMS = millis();
    bool currentPressState = (HIGH == digitalRead(BUTTON_PIN));

    // Check if new debounce state has occurred
    if (currentPressState != lastRawButtonPressState)
    {
        lastRawButtonPressState = currentPressState;
        lastRawButtonPressTimeMS = currentTimeMS;
    }

    // Check if debounce wait is over
    else if (currentTimeMS - lastRawButtonPressTimeMS > DEBOUNCE_MS)
    {
        // Increment the count
        totalPressCount++;

        // Update the state of the button
        lastDebouncePressTimeMS = currentTimeMS;
        currentDebouncedPressState = digitalRead(BUTTON_PIN);
    }

    return currentDebouncedPressState;
}


void setup()
{
    // LED
    pinMode(LED_PIN, HIGH);

    // Button
    pinMode(BUTTON_PIN, INPUT_PULLDOWN); // Active high
}

void loop()
{
    static bool isPlaying = false;
    static bool lastHigh = false;
    static uint32_t lastStateStartMS = 0;
    static uint32_t stateDurationMS = 0; // Time for on or off
    uint32_t currentTimeMS = millis();
    
    // Update the button state
    if (false == isButtonPressed())
    {
        // Check if 5s has expired
        if ((false == isPlaying) && (currentTimeMS - lastDebouncePressTimeMS > TIMEOUT_MS))
        {
            isPlaying = true;

            // Number of MS per blink cycle
            stateDurationMS = 500 / totalPressCount;

            // Update last states
            lastHigh = false;
            lastStateStartMS = 0; // Start next state right away
        }
    }
    else
    {
        // Check if a blink is playing
        if (false == isPlaying)
        {
            // reset state
            totalPressCount = 0;
            digitalWrite(LED_PIN, HIGH); // Disable LED
        }
    }

    // Play sequence
    if (isPlaying)
    {
        // Check if the state time expired
        if (currentTimeMS - lastStateStartMS > stateDurationMS)
        {
            // Toggle LED
            digitalWrite(LED_PIN, !lastHigh);
        }
    }
}
