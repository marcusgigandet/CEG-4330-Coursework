#pragma once

#include <cstdint>

namespace pins
{
    constexpr uint8_t LED{2};
    constexpr uint8_t BUTTON{12};
} // namespace Pins

/// Channel to use for PWM.
/// @note LEDC channel 0 is used by the camera clock, so don't use it for the LED
constexpr uint8_t PWM_CHANNEL = 4;
constexpr uint8_t PWM_RESOLUTION = 8;

constexpr uint32_t MAX_DUTY = (1U << PWM_RESOLUTION) - 1U;
constexpr float MIN_FREQUENCY_HZ = 1.0F;
constexpr float MAX_FREQUENCY_HZ = 40000.0F;
