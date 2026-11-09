#pragma once

#include <Arduino.h>

struct ButtonData
{
    /// Raw, current state of the button
    uint8_t rawState{HIGH};

    /// Debounced state of the button
    uint8_t debouncedState{HIGH};

    /// Start time of the debouncing
    uint32_t debounceStartTime{};

    /// Start time of the actual button press
    uint32_t pressStartTime{};

    /// Count of intentional button presses
    uint32_t pressCount{};

    /// Total duration of press
    uint32_t pressTime{};
};

extern ButtonData buttonData;

void handleButtonPress();
