/*
    Webpage controllable turntable code.
*/

#include "server.h"
#include "turntable.h"

void setup()
{
    Serial.begin(9600);

    // Initialize motor control pins
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(MOTOR_PINS[i], OUTPUT);
        digitalWrite(MOTOR_PINS[i], LOW);
    }

    // Initialize LED pin for error indication
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    // Initialize web server and load saved settings from flash
    initWebserver();
}

void loop()
{
    handleSettingsStorage();
    handleTurntable();
}