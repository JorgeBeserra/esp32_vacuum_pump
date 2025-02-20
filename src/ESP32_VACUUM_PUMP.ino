/*
 ESP32RET.ino

 Created: June 1, 2020
 Author: Collin Kidder

Copyright (c) 2014-2020 Collin Kidder, Michael Neuweiler

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "config.h"
#include <SPI.h>
#include <Preferences.h>
#include "SerialConsole.h"
#include "wifi_manager.h"
#include "lawicel.h"

byte i = 0;

uint32_t lastFlushMicros = 0;

bool markToggle[6];
uint32_t lastMarkTrigger = 0;

EEPROMSettings settings;
SystemSettings SysSettings;
Preferences nvPrefs;
char deviceName[20];
char otaHost[40];
char otaFilename[100];

uint8_t espChipRevision;

WiFiManager wifiManager;
SerialConsole console;

//initializes all the system EEPROM values. Chances are this should be broken out a bit but
//there is only one checksum check for all of them so it's simple to do it all here.
void loadSettings()
{
    Logger::console("Loading settings....");

    //for (int i = 0; i < NUM_BUSES; i++) canBuses[i] = nullptr;

    nvPrefs.begin(PREF_NAME, false);

    settings.logLevel = nvPrefs.getUChar("loglevel", 1); //info
    settings.wifiMode = nvPrefs.getUChar("wifiMode", 1); //Wifi defaults to creating an AP
    
    settings.systemType = 0; //nvPrefs.getUChar("systype", (espChipRevision > 2) ? 0 : 1); //0 = A0, 1 = EVTV ESP32

    if (settings.systemType == 0)
    {
        
        SysSettings.LED_CANTX = 26;
        SysSettings.LED_CANRX = 27;
        SysSettings.LED_LOGGING = 35;
        SysSettings.LED_CONNECTION_STATUS = 0;
        SysSettings.fancyLED = false;
        SysSettings.logToggle = false;
        SysSettings.txToggle = true;
        SysSettings.rxToggle = true;
        SysSettings.lawicelAutoPoll = false;
        SysSettings.lawicelMode = false;
        SysSettings.lawicellExtendedMode = false;
        SysSettings.lawicelTimestamping = false;
        SysSettings.numBuses = 1;
        SysSettings.isWifiActive = false;
        SysSettings.isWifiConnected = false;
        strcpy(deviceName, MACC_NAME);
        strcpy(otaHost, "");
        strcpy(otaFilename, "");
        pinMode(SysSettings.LED_CANTX, OUTPUT);
        pinMode(SysSettings.LED_CANRX, OUTPUT);
        digitalWrite(SysSettings.LED_CANTX, LOW);
        digitalWrite(SysSettings.LED_CANRX, LOW);
        delay(100);
        
    }

    if (nvPrefs.getString("SSID", settings.SSID, 32) == 0)
    {
        //strcpy(settings.SSID, deviceName);
        strcat(settings.SSID, "Sabidos");
   }

    if (nvPrefs.getString("wpa2Key", settings.WPA2Key, 64) == 0)
    {
        strcpy(settings.WPA2Key, "23Seb10STE5aNT");
    }

    if (nvPrefs.getString("mqttserver", settings.MQTT_server, 64) == 0)
    {
        strcpy(settings.MQTT_server, "23Seb10STE5aNT");
    }

    if (nvPrefs.getString("mqttuser", settings.MQTT_user, 64) == 0)
    {
        strcpy(settings.MQTT_user, "23Seb10STE5aNT");
    }

    if (nvPrefs.getString("mqttpass", settings.MQTT_pass, 64) == 0)
    {
        strcpy(settings.MQTT_pass, "23Seb10STE5aNT");
    }
    
    nvPrefs.end();

    Logger::setLoglevel((Logger::LogLevel)settings.logLevel);

}

void moveForward() {
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN3, HIGH);
    digitalWrite(MOTOR_B_IN4, LOW);
    analogWrite(MOTOR_A_EN, wheelSpeed);
    analogWrite(MOTOR_B_EN, wheelSpeed);
}

void turnRight() {
    digitalWrite(MOTOR_A_IN1, HIGH);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN3, LOW);
    digitalWrite(MOTOR_B_IN4, HIGH);
    analogWrite(MOTOR_A_EN, wheelSpeed);
    analogWrite(MOTOR_B_EN, wheelSpeed);
}

void turnLeft() {
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, HIGH);
    digitalWrite(MOTOR_B_IN3, HIGH);
    digitalWrite(MOTOR_B_IN4, LOW);
    analogWrite(MOTOR_A_EN, wheelSpeed);
    analogWrite(MOTOR_B_EN, wheelSpeed);
}

void stopMotors() {
    digitalWrite(MOTOR_A_IN1, LOW);
    digitalWrite(MOTOR_A_IN2, LOW);
    digitalWrite(MOTOR_B_IN3, LOW);
    digitalWrite(MOTOR_B_IN4, LOW);
    analogWrite(MOTOR_A_EN, 0);
    analogWrite(MOTOR_B_EN, 0);
    analogWrite(MOTOR_ESCOVAS, 0);
}

void startBrushes() {
    analogWrite(MOTOR_ESCOVAS, brushSpeed);
}

// Medir distância com sensor ultrassônico
float getDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH);
    return duration * 0.034 / 2;  // Distância em cm
}

// Medir nível da bateria (2S 18650)
int getBatteryLevel() {
    int adcValue = analogRead(BATTERY_PIN);  // Lê 0-4095
    float voltage = (adcValue / 4095.0) * 3.3 * 3.0;  // Divisor 2:1
    const float minVoltage = 6.0;  // 3.0V por célula
    const float maxVoltage = 8.4;  // 4.2V por célula
    int level = (voltage - minVoltage) / (maxVoltage - minVoltage) * 100;
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    return level;
}

// Interrupções dos encoders
void IRAM_ATTR encoderA() {
    if (digitalRead(ENCODER_A1) == digitalRead(ENCODER_A2)) encoderCountA++; else encoderCountA--;
}

void IRAM_ATTR encoderB() {
    if (digitalRead(ENCODER_B1) == digitalRead(ENCODER_B2)) encoderCountB++; else encoderCountB--;
}


// Callback MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == "homeassistant/vacuum/robo_aspirador/command") {
        if (message == "start") {
            vacuumState = "cleaning";
            startBrushes();
            Serial.println("Iniciando limpeza");
        } else if (message == "stop" || message == "pause") {
            vacuumState = "paused";
            stopMotors();
            Serial.println("Pausado");
        } else if (message == "return_to_base") {
            vacuumState = "returning";
            stopMotors();
            Serial.println("Retornando à base");
            delay(5000);  // Simulação
            vacuumState = "docked";
        }
    }
}

// Reconectar MQTT
void reconnect() {
    while (!client.connected()) {
        Serial.print("Conectando ao MQTT...");
        String clientId = "RoboAspirador-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), settings.MQTT_user, settings.MQTT_pass)) {
            Serial.println("conectado!");
            client.subscribe("homeassistant/vacuum/robo_aspirador/command");
        } else {
            Serial.print("falhou, rc=");
            Serial.print(client.state());
            delay(5000);
        }
    }
}

void setup()
{
    //delay(5000); //just for testing. Don't use in production

    espChipRevision = ESP.getChipRevision();

    Serial.begin(115200); //for production
    analogReadResolution(12);  // ADC 12 bits

    pinMode(MOTOR_A_EN, OUTPUT);
    pinMode(MOTOR_A_IN1, OUTPUT);
    pinMode(MOTOR_A_IN2, OUTPUT);
    pinMode(MOTOR_B_EN, OUTPUT);
    pinMode(MOTOR_B_IN3, OUTPUT);
    pinMode(MOTOR_B_IN4, OUTPUT);
    pinMode(MOTOR_ESCOVAS, OUTPUT);
    pinMode(TRIG_FRONTAL_ESQ, OUTPUT);
    pinMode(ECHO_FRONTAL_ESQ, INPUT);
    pinMode(TRIG_FRONTAL_DIR, OUTPUT);
    pinMode(ECHO_FRONTAL_DIR, INPUT);
    pinMode(TRIG_TRASEIRO, OUTPUT);
    pinMode(ECHO_TRASEIRO, INPUT);
    pinMode(ENCODER_A1, INPUT_PULLUP);
    pinMode(ENCODER_A2, INPUT_PULLUP);
    pinMode(ENCODER_B1, INPUT_PULLUP);
    pinMode(ENCODER_B2, INPUT_PULLUP);
    pinMode(BATTERY_PIN, INPUT);
   
    SysSettings.isWifiConnected = false;

    loadSettings();

    wifiManager.setup();
    
    Serial.println("");
    Serial.println("===================================");
    Serial.println("        Sabidos Vacuum Pump        ");
    Serial.println("===================================");
    Serial.println("");
    Serial.print("Build number: ");
    Serial.println(CFG_BUILD_NUM);

    SysSettings.lawicelMode = false;
    SysSettings.lawicelAutoPoll = false;
    SysSettings.lawicelTimestamping = false;
    SysSettings.lawicelPollCounter = 0;
    
}
 
/*
Send a fake frame out USB and maybe to file to show where the mark was triggered at. The fake frame has bits 31 through 3
set which can never happen in reality since frames are either 11 or 29 bit IDs. So, this is a sign that it is a mark frame
and not a real frame. The bottom three bits specify which mark triggered.
*/
void sendMarkTriggered(int which)
{
    //CAN_FRAME frame;
    //frame.id = 0xFFFFFFF8ull + which;
    //frame.extended = true;
    //frame.length = 0;
    //frame.rtr = 0;
    //canManager.displayFrame(frame, 0);
}

/*
Loop executes as often as possible all the while interrupts fire in the background.
The serial comm protocol is as follows:
All commands start with 0xF1 this helps to synchronize if there were comm issues
Then the next byte specifies which command this is.
Then the command data bytes which are specific to the command
Lastly, there is a checksum byte just to be sure there are no missed or duped bytes
Any bytes between checksum and 0xF1 are thrown away

Yes, this should probably have been done more neatly but this way is likely to be the
fastest and safest with limited function calls
*/
void loop()
{
    //uint32_t temp32;    
    bool isConnected = false;
    int serialCnt;
    uint8_t in_byte;   

    // Medir distâncias e bateria
    float distFrontEsq = getDistance(TRIG_FRONTAL_ESQ, ECHO_FRONTAL_ESQ);
    float distFrontDir = getDistance(TRIG_FRONTAL_DIR, ECHO_FRONTAL_DIR);
    float distTras = getDistance(TRIG_TRASEIRO, ECHO_TRASEIRO);
    batteryLevel = getBatteryLevel();

    // Calcular posição com encoders
    const float WHEEL_CIRCUMFERENCE = 20.0;  // Tamanho Roda
    const int PULSES_PER_REV = 20;  // Pulsos do Encoder
    static long lastCountA = 0, lastCountB = 0;
    float distanceA = (encoderCountA - lastCountA) * WHEEL_CIRCUMFERENCE / PULSES_PER_REV;
    float distanceB = (encoderCountB - lastCountB) * WHEEL_CIRCUMFERENCE / PULSES_PER_REV;
    lastCountA = encoderCountA;
    lastCountB = encoderCountB;
    posX += (distanceA + distanceB) / 2 * cos(angle * PI / 180);
    posY += (distanceA + distanceB) / 2 * sin(angle * PI / 180);
    angle += (distanceB - distanceA) / 20.0;  // 20 cm = distância entre rodas

    // Lógica de limpeza
    if (vacuumState == "cleaning") {
        if (distFrontEsq < 10.0 || distFrontDir < 10.0) {
            if (distFrontEsq < distFrontDir) {
                turnRight();
                delay(500);
            } else {
                turnLeft();
                delay(500);
            }
        } else {
            moveForward();
        }
        if (batteryLevel < 10) {
            vacuumState = "returning";
            stopMotors();
            delay(5000);
            vacuumState = "docked";
        }
    }

    // Publicar estado como JSON
    String stateJson = "{\"state\":\"" + vacuumState + "\","
    "\"battery_level\":" + String(batteryLevel) + ","
    "\"pos_x\":" + String(posX) + ","
    "\"pos_y\":" + String(posY) + ","
    "\"dist_front_esq\":" + String(distFrontEsq) + ","
    "\"dist_front_dir\":" + String(distFrontDir) + ","
    "\"dist_tras\":" + String(distTras) + "}";
    client.publish("homeassistant/vacuum/robo_aspirador/state", stateJson.c_str());

    delay(100);

}
