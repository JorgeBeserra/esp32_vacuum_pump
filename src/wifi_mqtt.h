// wifi_mqtt.h
#include <WiFi.h>
#include <PubSubClient.h>

class NetworkManager {
public:
    NetworkManager();
    void connect();
    void mqttLoop();
    void publishTelemetry(const String& data);
    
private:
    WiFiClient _espClient;
    PubSubClient _mqttClient;
    void _mqttCallback(char* topic, byte* payload, unsigned int length);
    void _reconnectMQTT();
};