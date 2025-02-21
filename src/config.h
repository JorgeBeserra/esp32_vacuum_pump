#pragma once

// Pinos GPIO
#define MOTOR1_PWM      25
#define MOTOR1_IN1      26
#define MOTOR1_IN2      27
#define MOTOR2_PWM      14
#define MOTOR2_IN3      12
#define MOTOR2_IN4      13

#define TRIGGER_FRONT1  5
#define ECHO_FRONT1     18
#define TRIGGER_FRONT2  19
#define ECHO_FRONT2     21
#define TRIGGER_REAR    22
#define ECHO_REAR       23

// Parâmetros do Sistema
#define SAFE_DISTANCE       20  // cm
#define MAX_MOTOR_SPEED     200
#define MIN_BATTERY_LEVEL   3.3 // volts

// WiFi
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_PASSWORD";

// MQTT
const char* MQTT_BROKER = "homeassistant.local";
const int MQTT_PORT = 1883;