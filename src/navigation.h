// navigation.h
#include "motor_control.h"
#include "sensor_manager.h"

class NavigationSystem {
public:
    NavigationSystem(MotorController& left, MotorController& right);
    void update();
    void emergencyStop();
    
private:
    MotorController& _leftMotor;
    MotorController& _rightMotor;
    void _avoidObstacle();
};