// navigation.cpp
#include "navigation.h"

NavigationSystem::NavigationSystem(MotorController& left, MotorController& right) 
    : _leftMotor(left), _rightMotor(right) {}

void NavigationSystem::update() {
    // Implementação do algoritmo Bug2
    if(_leftSensor.getDistance() < SAFE_DISTANCE || 
       _rightSensor.getDistance() < SAFE_DISTANCE) {
        _avoidObstacle();
    } else {
        _leftMotor.setSpeed(200);
        _rightMotor.setSpeed(200);
    }
}