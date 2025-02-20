/*
 * SerialConsole.cpp
 *
 Copyright (c) 2014-2018 Collin Kidder

 Shamelessly copied from the version in GEVCU

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

#include "SerialConsole.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <Preferences.h>
#include "config.h"
#include "sys_io.h"
#include "lawicel.h"

SerialConsole::SerialConsole()
{
    init();
}

void SerialConsole::init()
{
    //State variables for serial console
    ptrBuffer = 0;
    state = STATE_ROOT_MENU;
}

void SerialConsole::printMenu()
{
    char buff[80];
    //Show build # here as well in case people are using the native port and don't get to see the start up messages
    Serial.print("Build number: ");
    Serial.println(CFG_BUILD_NUM);
    Serial.println("System Menu:");
    Serial.println();
    Serial.println("Enable line endings of some sort (LF, CR, CRLF)");
    Serial.println();
    Serial.println("Short Commands:");
    Serial.println("h = help (displays this message)");
    Serial.println("R = reset to factory defaults");
    Serial.println("s = Start logging to file");
    Serial.println("S = Stop logging to file");
    Serial.println();
    Serial.println("Config Commands (enter command=newvalue). Current values shown in parenthesis:");
    Serial.println();

    Logger::console("SYSTYPE=%i - Set board type (0=Macchina A0, 1=EVTV ESP32 Board 2=Macchina A5)", settings.systemType);
    Logger::console("LOGLEVEL=%i - set log level (0=debug, 1=info, 2=warn, 3=error, 4=off)", settings.logLevel);
    Serial.println();

    Logger::console("LAWICEL=%i - Set whether to accept LAWICEL commands (0 = Off, 1 = On)", settings.enableLawicel);
    Serial.println();

    Logger::console("WIFIMODE=%i - Set mode for WiFi (0 = Wifi Off, 1 = Connect to AP, 2 = Create AP", settings.wifiMode);
    Logger::console("SSID=%s - Set SSID to either connect to or create", (char *)settings.SSID);
    Logger::console("WPA2KEY=%s - Either passphrase or actual key", (char *)settings.WPA2Key);
}

void SerialConsole::rcvCharacter(uint8_t chr)
{
    if (chr == 10 || chr == 13) { //command done. Parse it.
        handleConsoleCmd();
        ptrBuffer = 0; //reset line counter once the line has been processed
    } else {
        cmdBuffer[ptrBuffer++] = (unsigned char) chr;
        if (ptrBuffer > 79)
            ptrBuffer = 79;
    }
}

void SerialConsole::handleConsoleCmd()
{
    if (state == STATE_ROOT_MENU) {
        if (ptrBuffer == 1) {
            //command is a single ascii character
            handleShortCmd();
        } else { //at least two bytes
            boolean equalSign = false;
            for (int i = 0; i < ptrBuffer; i++) if (cmdBuffer[i] == '=') equalSign = true;
            cmdBuffer[ptrBuffer] = 0; //make sure to null terminate
            if (equalSign) handleConfigCmd();
            else if (settings.enableLawicel) lawicel.handleLongCmd(cmdBuffer);
        }
        ptrBuffer = 0; //reset line counter once the line has been processed
    }
}

void SerialConsole::handleShortCmd()
{
    uint8_t val;

    switch (cmdBuffer[0]) {
    //non-lawicel commands
    case 'h':
    case '?':
    case 'H':
        printMenu();
        break;
    case 'R': //reset to factory defaults.
        nvPrefs.begin(PREF_NAME, false);
        nvPrefs.clear();
        nvPrefs.end();        
        Logger::console("Power cycle to reset to factory defaults");
        break;
    case 'P': //reset to factory defaults.
        Logger::console("O");
        break;
    case '~':
        Serial.println("DEBUGGING MODE!");        
        break;
    case '`':
        Serial.println("Normal mode");
        break;    
    default:
        if (settings.enableLawicel) lawicel.handleShortCmd(cmdBuffer[0]);
        break;
    }
}

void SerialConsole::handleConfigCmd()
{
    int i;
    int newValue;
    char *newString;
    bool writeEEPROM = false;
    bool writeDigEE = false;
    char *dataTok;

    //Logger::debug("Cmd size: %i", ptrBuffer);
    if (ptrBuffer < 6)
        return; //4 digit command, =, value is at least 6 characters
    cmdBuffer[ptrBuffer] = 0; //make sure to null terminate
    String cmdString = String();
    unsigned char whichEntry = '0';
    i = 0;

    while (cmdBuffer[i] != '=' && i < ptrBuffer)
    {
        cmdString.concat(String(cmdBuffer[i++]));
    }
    i++; //skip the =
    if (i >= ptrBuffer)
    {
        Logger::console("");
        return; //or, we could use this to display the parameter instead of setting
    }

    // strtol() is able to parse also hex values (e.g. a string "0xCAFE"), useful for enable/disable by device id
    newValue = strtol((char *) (cmdBuffer + i), NULL, 0); //try to turn the string into a number
    newString = (char *)(cmdBuffer + i); //leave it as a string

    cmdString.toUpperCase();

    if (cmdString.startsWith("CANEN")) 
    {
        
    } else if (cmdString == String("LOGLEVEL")) {
        switch (newValue) {
        case 0:
            Logger::setLoglevel(Logger::Debug);
            settings.logLevel = 0;
            Logger::console("setting loglevel to 'debug'");
            writeEEPROM = true;
            break;
        case 1:
            Logger::setLoglevel(Logger::Info);
            settings.logLevel = 1;
            Logger::console("setting loglevel to 'info'");
            writeEEPROM = true;
            break;
        case 2:
            Logger::console("setting loglevel to 'warning'");
            settings.logLevel = 2;
            Logger::setLoglevel(Logger::Warn);
            writeEEPROM = true;
            break;
        case 3:
            Logger::console("setting loglevel to 'error'");
            settings.logLevel = 3;
            Logger::setLoglevel(Logger::Error);
            writeEEPROM = true;
            break;
        case 4:
            Logger::console("setting loglevel to 'off'");
            settings.logLevel = 4;
            Logger::setLoglevel(Logger::Off);
            writeEEPROM = true;
            break;
        }

    } else {
        Logger::console("Unknown command");
    }
    if (writeEEPROM) {
        nvPrefs.begin(PREF_NAME, false);

        char buff[80];
        nvPrefs.putBool("enable-bt", settings.enableBT);
        nvPrefs.putBool("enableLawicel", settings.enableLawicel);
        nvPrefs.putUChar("loglevel", settings.logLevel);
        nvPrefs.putUChar("systype", settings.systemType);
        nvPrefs.putUChar("wifiMode", settings.wifiMode);
        nvPrefs.putString("SSID", settings.SSID);
        nvPrefs.putString("wpa2Key", settings.WPA2Key);
        nvPrefs.putString("btname", settings.btName);
        nvPrefs.end();
    }
} 

//CAN0FILTER%i=%%i,%%i,%%i,%%i (ID, Mask, Extended, Enabled)", i);
bool SerialConsole::handleFilterSet(uint8_t bus, uint8_t filter, char *values)
{
    if (filter < 0 || filter > 7) return false;
    if (bus < 0 || bus > 1) return false;

    //there should be four tokens
    char *idTok = strtok(values, ",");
    char *maskTok = strtok(NULL, ",");
    char *extTok = strtok(NULL, ",");
    char *enTok = strtok(NULL, ",");

    if (!idTok) return false; //if any of them were null then something was wrong. Abort.
    if (!maskTok) return false;
    if (!extTok) return false;
    if (!enTok) return false;

    int idVal = strtol(idTok, NULL, 0);
    int maskVal = strtol(maskTok, NULL, 0);
    int extVal = strtol(extTok, NULL, 0);
    int enVal = strtol(enTok, NULL, 0);

    Logger::console("Setting CAN%iFILTER%i to ID 0x%x Mask 0x%x Extended %i Enabled %i", bus, filter, idVal, maskVal, extVal, enVal);

    if (bus == 0) {
        //settings.CAN0Filters[filter].id = idVal;
        //settings.CAN0Filters[filter].mask = maskVal;
        //settings.CAN0Filters[filter].extended = extVal;
        //settings.CAN0Filters[filter].enabled = enVal;
        //CAN0.setRXFilter(filter, idVal, maskVal, extVal);
    } else if (bus == 1) {
        //settings.CAN1Filters[filter].id = idVal;
        //settings.CAN1Filters[filter].mask = maskVal;
        //settings.CAN1Filters[filter].extended = extVal;
        //settings.CAN1Filters[filter].enabled = enVal;
        //CAN1.setRXFilter(filter, idVal, maskVal, extVal);
    }

    return true;
}

void SerialConsole::printBusName(int bus) {
    switch (bus) {
    case 0:
        Serial.print("CAN0");
        break;
    case 1:
        Serial.print("CAN1");
        break;
    default:
        Serial.print("UNKNOWN");
        break;
    }
}
