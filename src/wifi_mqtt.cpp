// wifi_mqtt.cpp
#include "wifi_mqtt.h"
#include "config.h"

NetworkManager::NetworkManager() : _mqttClient(_espClient) {
    _mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    _mqttClient.setCallback([this](char* t, byte* p, unsigned int l) {
        this->_mqttCallback(t, p, l);
    });
}

void NetworkManager::connect() {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    _reconnectMQTT();
}

void NetworkManager::_reconnectMQTT() {
    while(!_mqttClient.connected()) {
        if(_mqttClient.connect("VacuumRobot", "mqtt_user", "mqtt_pass")) {
            _mqttClient.subscribe("home/robot/control");
        }
        delay(5000);
    }
}