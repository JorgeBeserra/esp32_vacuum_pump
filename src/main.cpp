#include <Arduino.h>
#include "config.h"
#include "motor_control.h"
//#include "sensor_manager.h"
//#include "wifi_mqtt.h"
//#include "navigation.h"

// Definições das variáveis globais
const char* WIFI_SSID = "YOUR_SSID";
const char* WIFI_PASS = "YOUR_PASSWORD";
const char* MQTT_BROKER = "homeassistant.local";
const int MQTT_PORT = 1883;

MotorController leftMotor(MOTOR1_PWM, MOTOR1_IN1, MOTOR1_IN2, MOTOR_LEFT_CHANNEL);
MotorController rightMotor(MOTOR2_PWM, MOTOR2_IN3, MOTOR2_IN4, MOTOR_RIGHT_CHANNEL);
//NetworkManager network;
//NavigationSystem navSystem(leftMotor, rightMotor);

void setupOTA() {
    // ArduinoOTA
    //   .onStart([]() {
    //     String type;
    //     if (ArduinoOTA.getCommand() == U_FLASH)
    //       type = "sketch";
    //     else
    //       type = "filesystem";
    //     Serial.println("Start updating " + type);
    //   })
    //   .onEnd([]() {
    //     Serial.println("\nEnd");
    //   })
    //   .onProgress([](unsigned int progress, unsigned int total) {
    //     Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    //   })
    //   .onError([](ota_error_t error) {
    //     Serial.printf("Error[%u]: ", error);
    //     if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    //     else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    //     else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    //     else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    //     else if (error == OTA_END_ERROR) Serial.println("End Failed");
    //   });
  
    // ArduinoOTA.begin();
  }

void setup() {
    Serial.begin(115200);
    //setupOTA();
    //network.connect();
    Serial.println("Iniciando!");
    
    // Exemplo de uso
    leftMotor.setSpeed(200);  // Move para frente
    rightMotor.setSpeed(200);
    delay(1000);

    leftMotor.setSpeed(-150); // Move para trás
    rightMotor.setSpeed(-150);
    delay(1000);

    leftMotor.brake();        // Freia
    rightMotor.brake();
}

void loop() {
    //ArduinoOTA.handle();
    //network.mqttLoop();
    //navSystem.update();
    //delay(100);
}