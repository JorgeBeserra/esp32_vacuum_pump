/*
 * config.h
 *
 * allows the user to configure static parameters.
 *
 * Note: Make sure with all pin defintions of your hardware that each pin number is
 *       only defined once.

 Copyright (c) 2013-2020 Collin Kidder, Michael Neuweiler, Charles Galpin

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

#ifndef CONFIG_H_
#define CONFIG_H_

#include <WiFi.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

//size to use for buffering writes to USB. On the ESP32 we're actually talking TTL serial to a TTL<->USB chip
#define SER_BUFF_SIZE       1024

//Buffer for CAN frames when sending over wifi. This allows us to build up a multi-frame packet that goes
//over the air all at once. This is much more efficient than trying to send a new TCP/IP packet for each and every
//frame. It delays frames from getting to the other side a bit but that's life.
//Probably don't set this over 2048 as the default packet size for wifi is 2312 including all overhead.
#define WIFI_BUFF_SIZE      2048

//Number of microseconds between hard flushes of the serial buffer (if not in wifi mode) or the wifi buffer (if in wifi mode)
//This keeps the latency more consistent. Otherwise the buffer could partially fill and never send.
#define SER_BUFF_FLUSH_INTERVAL 20000

#define CFG_BUILD_NUM   1
#define CFG_VERSION "Alpha Feb 20 2025"
#define PREF_NAME   "ESP32_VACUUM_PUMP"
#define EVTV_NAME   "ESP32_VACUUM_PUMP"
#define MACC_NAME   "VACPUMP"

#define MARK_LIMIT  6   //# of our analog input pins to use for marking. Defaults to all of them. Send voltage to pin to trigger it

#define NUM_ANALOG  NUM_ANALOG_INPUTS   // we get the number of analogue inputs from variant.h
#define NUM_DIGITAL 6   // Not currently using digital pins on the ESP32
#define NUM_OUTPUT  6   // Ditto

#define NUM_BUSES   5   //max # of buses supported by any of the supported boards

//It's not even used on this hardware currently. But, slows down the blinks to make them more visible
#define BLINK_SLOWNESS  100 

#define A0_LED_PIN     33
#define A0_NUM_LEDS    0
#define A5_LED_PIN     25
#define A5_NUM_LEDS    0
#define BRIGHTNESS  190

#define SW_EN     2
#define SW_MODE0  26
#define SW_MODE1  27

#define MOSFET_PIN 25  // GPIO para controle do MOSFET
#define CAN_INT_PIN GPIO_NUM_4
#define TIMEOUT_WARNING 10000  // 1 minuto sem atividade CAN para enviar o aviso -> Temporario em 30 segundos
#define TIMEOUT_SHUTDOWN 30000 // 5 minutos para desligamento total -> Temporario em 1 minuto

// Pinos dos motores
#define MOTOR_A_EN 19  // Roda esquerda PWM
#define MOTOR_A_IN1 22
#define MOTOR_A_IN2 23
#define MOTOR_B_EN 20  // Roda direita PWM
#define MOTOR_B_IN3 24
#define MOTOR_B_IN4 25
#define MOTOR_ESCOVAS 26  // Escovas PWM

// Pinos dos sensores ultrassônicos
#define TRIG_FRONTAL_ESQ 5
#define ECHO_FRONTAL_ESQ 18
#define TRIG_FRONTAL_DIR 6
#define ECHO_FRONTAL_DIR 19
#define TRIG_TRASEIRO 7
#define ECHO_TRASEIRO 21

// Pino ADC para bateria 18650 (2S)
#define BATTERY_PIN 36  // GPIO36 (ADC1_0)

// Pinos dos encoders
#define ENCODER_A1 32  // Roda esquerda Canal A
#define ENCODER_A2 33  // Roda esquerda Canal B
#define ENCODER_B1 34  // Roda direita Canal A
#define ENCODER_B2 35  // Roda direita Canal B

//How many devices to allow to connect to our WiFi telnet port?
#define MAX_CLIENTS 1

struct EEPROMSettings {
    uint8_t logLevel; //Level of logging to output on serial line
    uint8_t systemType; //0 = A0RET, 1 = EVTV ESP32 Board, 2 = Macchine 5-CAN board
    uint8_t wifiMode; //0 = don't use wifi, 1 = connect to an AP, 2 = Create an AP
    char SSID[32];     //null terminated string for the SSID
    char WPA2Key[64]; //Null terminated string for the key. Can be a passphase or the actual key
    char MQTT_server[32];     //null terminated string for the SSID
    char MQTT_user[32];     //null terminated string for the SSID
    char MQTT_pass[32];     //null terminated string for the SSID
} __attribute__((__packed__));

struct SystemSettings {
    uint8_t LED_CANTX;
    uint8_t LED_CANRX;
    uint8_t LED_LOGGING;
    uint8_t LED_CONNECTION_STATUS;
    boolean fancyLED;
    boolean txToggle; //LED toggle values
    boolean rxToggle;
    boolean logToggle;
    boolean lawicelMode;
    boolean lawicellExtendedMode;
    boolean lawicelAutoPoll;
    boolean lawicelTimestamping;
    int lawicelPollCounter;
    boolean lawicelBusReception[NUM_BUSES]; //does user want to see messages from this bus?
    int8_t numBuses; //number of buses this hardware currently supports.
    WiFiClient clientNodes[MAX_CLIENTS];
    WiFiClient wifiOBDClients[MAX_CLIENTS];
    boolean isWifiConnected;
    boolean isWifiActive;
};

class SerialConsole;

extern EEPROMSettings settings;
extern SystemSettings SysSettings;
extern Preferences nvPrefs;


extern SerialConsole console;

extern char deviceName[20];
extern char otaHost[40];
extern char otaFilename[100];

//WiFiClient espClient;
//PubSubClient client(espClient);
AsyncWebServer server(80);
String vacuumState = "idle";  // Estados: idle, cleaning, paused, returning, docked, error
float posX = 0.0, posY = 0.0, angle = 0.0;  // Posição em cm e graus
volatile long encoderCountA = 0, encoderCountB = 0;
const int wheelSpeed = 128;  // PWM ~50%
const int brushSpeed = 255;  // Escovas 100%
int batteryLevel = 100;  // Nível da bateria em %

#endif /* CONFIG_H_ */
