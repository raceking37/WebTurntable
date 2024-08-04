#include "server.h"
#include "turntable.h"

void setup()
{
    Serial.begin(9600);

    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(MOTOR_PINS[i], OUTPUT);
        digitalWrite(MOTOR_PINS[i], LOW);
    }

    initWebserver();
}

void loop()
{
    handleTurntable();
}