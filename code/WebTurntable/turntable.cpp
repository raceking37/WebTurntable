#include "turntable.h"

// Global turntable settings structure
TurntableSettings g_turntable = {};

namespace
{
    // Motor coil activation patterns for half-stepping mode (8 phases)
    constexpr uint8_t HALF_STEPPING_PATTERN[8][4] = 
    {
        {1, 0, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 1},
        {0, 0, 0, 1},
        {1, 0, 0, 1}
    };

    // Motor coil activation patterns for full-stepping mode (4 phases)
    constexpr uint8_t FULL_STEPPING_PATTERN[4][4] = 
    {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    };

    // Convert slider position (0-100) to motor step delay (microseconds)
    uint16_t convertSpeedToDelay(const uint16_t sliderSpeed)
    {
        constexpr uint16_t kDelayMultiplier = (MAX_DELAY - MIN_DELAY) / TOTAL_SLIDER_STEPS;
        return MAX_DELAY - (kDelayMultiplier * sliderSpeed);
    }

    // Execute one half-stepping phase
    // Applies activation pattern and advances to next coil sequence
    void stepHalf(const uint16_t stepDelay, const RotateDirection dir)
    {
        static uint8_t counter = 0;

        for (uint8_t i = 0; i < 4; i++)
        {
            digitalWrite(MOTOR_PINS[i], HALF_STEPPING_PATTERN[counter][i]);
        }
        delayMicroseconds(stepDelay);

        counter = dir == RotateDirection::CW ? (counter + 7) % 8 : (counter + 1) % 8;
    }

    // Execute one full-stepping phase
    // Applies activation pattern and advances to next coil sequence
    void stepFull(uint16_t stepDelay, RotateDirection dir)
    {
        static uint8_t counter = 0;

        for (uint8_t i = 0; i < 4; i++)
        {
            digitalWrite(MOTOR_PINS[i], FULL_STEPPING_PATTERN[counter][i]);
        }
        delayMicroseconds(stepDelay);
        
        counter = dir == RotateDirection::CW ? (counter + 3) % 4 : (counter + 1) % 4;
    }
}

// Main turntable control loop
void handleTurntable()
{    
    // Stop motor if speed is zero or direction is OFF
    if (g_turntable.sliderSpeed == 0 || g_turntable.direction == RotateDirection::OFF)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            digitalWrite(MOTOR_PINS[i], LOW);
        }
        return;
    }

    uint16_t stepDelay = convertSpeedToDelay(g_turntable.sliderSpeed);

    switch (g_turntable.mode)
    {
        case RotateMode::HALF_STEPPING:
            stepHalf(stepDelay / 2, g_turntable.direction);
            break;

        case RotateMode::FULL_STEPPING:
            stepFull(stepDelay, g_turntable.direction);
            break;

        default:
            break;
    }
}