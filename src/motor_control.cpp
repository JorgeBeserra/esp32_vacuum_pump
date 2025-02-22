#include "motor_control.h"
#include "config.h"

// Construtor
MotorController::MotorController(uint8_t pwmPin, uint8_t in1, uint8_t in2, uint8_t channel)
    : _pwmPin(pwmPin), _in1(in1), _in2(in2), _channel(channel) {
    
    if (_channel > 15) {
        Serial.printf("Erro: Canal PWM %d inválido (máx 15)\n", _channel);
        _channel = 0;
    }
    
    // Configura pinos
    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
    pinMode(_pwmPin, OUTPUT);

    // Configura PWM
    _setupPWM();
}

// Configuração do PWM
void MotorController::_setupPWM() {
    if (ledcSetup(_channel, 5000, 8) == 0) {
        Serial.printf("Erro: Falha ao configurar PWM no canal %d\n", _channel);
        return;
    }
    ledcAttachPin(_pwmPin, _channel);
    ledcWrite(_channel, 0);
}

// Define a velocidade do motor
void MotorController::setSpeed(int speed) {
    speed = constrain(speed, -MAX_MOTOR_SPEED, MAX_MOTOR_SPEED);
    if (_inverted) speed = -speed;
    if (speed > 0) {
        digitalWrite(_in1, HIGH);
        digitalWrite(_in2, LOW);
    } else if (speed < 0) {
        digitalWrite(_in1, LOW);
        digitalWrite(_in2, HIGH);
    } else {
        stop();
    }
    ledcWrite(_channel, abs(speed));
    _currentSpeed = speed;
}
// Para o motor (free running)
void MotorController::stop() {
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);
    ledcWrite(_channel, 0);
    _currentSpeed = 0;
}

// Freia o motor (short brake)
void MotorController::brake() {
    digitalWrite(_in1, HIGH);
    digitalWrite(_in2, HIGH);
    ledcWrite(_channel, 255);
    delay(100);  // Freio breve
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);
    ledcWrite(_channel, 0);
    _currentSpeed = 0;
}

// Inverte a direção do motor
void MotorController::setInverted(bool inverted) {
    _inverted = inverted;
    if (_currentSpeed != 0) {
        setSpeed(_currentSpeed); // Reaplica a velocidade com a nova direção
    }
}

// Retorna a velocidade atual
int MotorController::getCurrentSpeed() const {
    return _currentSpeed;
}

// Verifica se o motor está em movimento
bool MotorController::isRunning() const {
    return _currentSpeed != 0 && (digitalRead(_in1) != digitalRead(_in2));
}