#ifndef TURNTABLE_H
#define TURNTABLE_H

// Turntable pin definitions
const uint8_t MOTOR_PINS[] = {D5, D6, D7, D8};
const uint8_t LED_PIN = D4;

// Describes the usable stepping delay range
#define MAX_DELAY 20000
#define MIN_DELAY 1800

// Webpage slider step count
#define TOTAL_SLIDER_STEPS 100

// Delay change per slider increment (in microseconds)
#define DELAY_MULTIPLIER ((MAX_DELAY - MIN_DELAY)/TOTAL_SLIDER_STEPS)

enum RotateDirection
{
    CW,
    OFF,
    CCW
};

enum RotateMode
{
    FULL_STEPPING,
    HALF_STEPPING
};

struct Turntable
{
    uint8_t direction;
    uint16_t delayValue;
    uint16_t sliderSpeed;
    uint8_t mode;
};

extern Turntable gTurntable;

const uint8_t HALF_STEPPING_PATTERN[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1}
};

const uint8_t FULL_STEPPING_PATTERN[4][4] = {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

void stepHalf(uint16_t stepDelay, bool rotateCW);
void stepFull(uint16_t stepDelay, bool rotateCW);
void handleTurntable();

#endif // TURNTABLE_H