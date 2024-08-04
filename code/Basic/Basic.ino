/*
    Simple example code to get the Turntable spinning.
*/

const uint8_t MOTOR_PINS[] = {D5, D6, D7, D8};
const uint8_t LED_PIN = D4;

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

void setup()
{
    // Configure the motor driver connection pins
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(MOTOR_PINS[i], OUTPUT);
        digitalWrite(MOTOR_PINS[i], LOW);
    }
}

void loop()
{
    /* 
        Make a step (half-stepping) every 2000 microseconds
        Rotate the motor clockwise
    */
    stepHalf(2000, true);
}

/*
    The 28byj-48 has 2038 steps per revolution in full stepping mode.
    The function below drives it in half-stepping mode, which will need
    4076 steps in order to make one full rotation.
*/
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