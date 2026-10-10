// Use a GPIO pin and interrupt to measure the frequency and duty cycle of an input pin. 
// Assume that there is a sensor that is providing a 1Hz to 1KHz input square wave with 
// variable duty cycle on an input pin.
//
// You must print two quantities to the serial monitor, only if they change:
//
// The frequency of the input signal.
// The duty cycle of the input signal.

#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include "esp_attr.h"
#include <Arduino.h>

// Constants
constexpr uint32_t BAUD_RATE = 9'600;

// Pins
constexpr uint8_t LED_PIN = 2;
constexpr uint8_t SENSOR_PIN = 23;

// Globals

// ISR
volatile bool hasDutyCycleChanged = false;

volatile uint32_t frameCount = 0;
volatile uint32_t lastLowUS = 0;
volatile uint32_t lastHighUS = 0;
volatile uint32_t timeLowUS = 0;
volatile uint32_t timeHighUS = 0;

void IRAM_ATTR edgeISR()
{
    uint8_t currentRead = digitalRead(SENSOR_PIN);
    uint32_t currentTimeUS = micros();
    uint32_t stateDurationUS = 0; // Length of time low / high

    // Check if time low changed
    if (HIGH == currentRead)
    {
        stateDurationUS = currentTimeUS - lastLowUS;

        // Set bool if state changed
        hasDutyCycleChanged = stateDurationUS == timeLowUS;

        // Update high state start
        lastHighUS = currentTimeUS;
    }
    // Check if time high changed
    else
    {
        stateDurationUS = currentTimeUS - lastHighUS;

        // Set bool if state changed
        hasDutyCycleChanged = stateDurationUS == timeHighUS;

        // Update low state start
        lastLowUS = currentTimeUS;
    }

    // Update the total count for this frame
    frameCount = frameCount + 1;
}

void setup()
{
    // Serial
    Serial.begin(BAUD_RATE);

    // LED
    pinMode(LED_PIN, HIGH);

    // Sensor
    pinMode(SENSOR_PIN, INPUT); // Sensor always defines sequare wave
    attachInterrupt(SENSOR_PIN, edgeISR, CHANGE);
}

void loop()
{
    static uint32_t lastTimeMS = 0;
    static uint32_t lastFrameCount = 0;
    uint32_t currentTimeMS = millis();

    // Reset frequency frame every second
    if (currentTimeMS - lastTimeMS >= 1'000)
    {
        // Print the frequency if it changed
        if (lastFrameCount != frameCount)
        {
            Serial.printf("Frequency: %.2f", 1'000'000 / (float)frameCount);
        }

        frameCount = 0;
    }

    // Print the duty cycle
    if (hasDutyCycleChanged)
    {
        Serial.printf("Duty Cycle: %.2f", (float)timeHighUS / (float)(timeLowUS + timeHighUS));
        
        // Reset state
        hasDutyCycleChanged = false;
    }
}
