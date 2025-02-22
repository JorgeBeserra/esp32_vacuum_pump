#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

class MotorController {
public:
    // Constructor
    MotorController(uint8_t pwmPin, uint8_t in1, uint8_t in2, uint8_t channel);

    // Controle de velocidade
    void setSpeed(int speed);
    void stop();
    void brake();

    // Configuracao
    void setInverted(bool inverted);
    int getCurrentSpeed() const;
    bool isRunning() const;

private:
    uint8_t _pwmPin;
    uint8_t _in1;
    uint8_t _in2;
    uint8_t _channel;
    bool _inverted = false;
    int _currentSpeed = 0;

    // Configuração do PWM
    void _setupPWM();
};

#endif