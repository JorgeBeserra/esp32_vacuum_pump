#include <Arduino.h>
#include <driver/ledc.h>

class MotorController {
public:
    MotorController(uint8_t pwmPin, uint8_t in1, uint8_t in2);
    void setSpeed(int speed);
    void brake();
    void freeRun();

private:
    uint8_t _pwmPin;
    uint8_t _in1;
    uint8_t _in2;
    ledc_channel_config_t _pwmConfig;
};