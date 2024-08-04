#include "Arduino.h"
#include "turntable.h"

Turntable gTurntable = {1, 0, 0, 1};

// Make one step in half stepping mode
void stepHalf(uint16_t stepDelay, bool rotateCW)
{
    static uint8_t counter = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(MOTOR_PINS[i], HALF_STEPPING_PATTERN[counter][i]);
    }
    delayMicroseconds(stepDelay);

    counter = rotateCW ? (counter + 7) % 8 : (counter + 1) % 8;
}


// Make one step in full stepping mode
void stepFull(uint16_t stepDelay, bool rotateCW)
{
    static uint8_t counter = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        digitalWrite(MOTOR_PINS[i], FULL_STEPPING_PATTERN[counter][i]);
    }
    delayMicroseconds(stepDelay);
    
    counter = rotateCW ? (counter + 3) % 4 : (counter + 1) % 4;
}


// Do turntable things
void handleTurntable()
{    
    if (gTurntable.sliderSpeed == 0 || gTurntable.direction == RotateDirection::OFF)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            digitalWrite(MOTOR_PINS[i], LOW);
        }
        return;
    }

    bool rotateCW = gTurntable.direction == RotateDirection::CW ? true : false;

    switch (gTurntable.mode)
    {
        case RotateMode::HALF_STEPPING:
            stepHalf(gTurntable.delayValue/2, rotateCW);
            break;

        case RotateMode::FULL_STEPPING:
            stepFull(gTurntable.delayValue, rotateCW);
            break;

        default:
            break;
    }
}