#ifndef TURNTABLE_H
#define TURNTABLE_H

#include <Arduino.h>

// Turntable pin definitions
const uint8_t MOTOR_PINS[] = {D5, D6, D7, D8};
const uint8_t LED_PIN = D4;

// Describes the usable stepping delay range
#define MAX_DELAY 20000
#define MIN_DELAY 1800

// Webpage slider step count
#define TOTAL_SLIDER_STEPS 100

// Rotation direction states
enum RotateDirection : uint8_t
{
    CW = 0,
    OFF = 1,
    CCW = 3
};

// Stepping mode determines the coil activation pattern and step sequence
enum RotateMode : uint8_t
{
    FULL_STEPPING = 0,
    HALF_STEPPING = 1
};

// Struct to hold the current turntable settings
struct TurntableSettings
{
    static constexpr uint32_t MAGIC = 0x5454424C;

    uint32_t magic              = MAGIC;
    RotateDirection direction   = RotateDirection::OFF;
    uint16_t sliderSpeed        = 0;
    RotateMode mode             = RotateMode::FULL_STEPPING;
};

// Global instance holding the turntable settings
extern TurntableSettings g_turntable;

// Main turntable control loop
void handleTurntable();

#endif // TURNTABLE_H