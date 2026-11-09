#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include <Arduino.h>

// Pins
constexpr uint8_t LED_PIN = 2;
constexpr uint8_t BUTTON_PIN = 23;

// Constants
constexpr uint32_t DEBOUNCE_MS = 1'000;

void setup()
{
    // Serial
    Serial.begin(9'600);

    // LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);    // Disable

    // Button
    pinMode(BUTTON_PIN, INPUT_PULLDOWN);
}

bool isButtonPressed()
{
    // Saved button states
    static uint32_t lastDebounceStart = 0;
    static uint8_t lastReadVal = digitalRead(BUTTON_PIN);
    static uint8_t buttonReadVal = lastReadVal;

    // Current button states
    uint32_t currentTime = millis();
    uint8_t readVal = digitalRead(BUTTON_PIN);

    // Check if new debounce started
    if (readVal != lastReadVal)
    {
        // Update state
        lastDebounceStart = currentTime;
        lastReadVal = readVal;
    }

    // Check for debounce duration
    else if (currentTime - lastDebounceStart >= DEBOUNCE_MS)
    {
        // Update debounced press state
        buttonReadVal = lastReadVal;
    }

    // Return the button press state
    return HIGH == buttonReadVal;
}

void loop()
{
    if (isButtonPressed())
    {
        Serial.println("Pressed");
    }
}