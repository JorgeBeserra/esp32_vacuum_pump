// motor_control.cpp
#include "motor_control.h"

MotorController::MotorController(uint8_t pwmPin, uint8_t in1, uint8_t in2) 
    : _pwmPin(pwmPin), _in1(in1), _in2(in2) {
    
    ledc_timer_config_t timerConf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timerConf);

    _pwmConfig = {
        .gpio_num = _pwmPin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&_pwmConfig);

    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
}

void MotorController::setSpeed(int speed) {
    speed = constrain(speed, -255, 255);
    digitalWrite(_in1, speed > 0);
    digitalWrite(_in2, speed < 0);
    ledc_set_duty(_pwmConfig.speed_mode, _pwmConfig.channel, abs(speed));
    ledc_update_duty(_pwmConfig.speed_mode, _pwmConfig.channel);
}