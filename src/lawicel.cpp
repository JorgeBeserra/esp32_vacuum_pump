/*
Implements the lawicel protocol.
*/

#include "lawicel.h"
#include "config.h"

void LAWICELHandler::handleShortCmd(char cmd)
{
    switch (cmd)
    {
    case 'O': //LAWICEL open canbus port

        Serial.write(13); //send CR to mean "ok"
        SysSettings.lawicelMode = true;
        break;
    case 'C': //LAWICEL close canbus port (First one)
        Serial.write(13); //send CR to mean "ok"
        break;
    case 'L': //LAWICEL open canbus port in listen only mode
       
        Serial.write(13); //send CR to mean "ok"
        SysSettings.lawicelMode = true;
        break;
    case 'P': //LAWICEL - poll for one waiting frame. Or, just CR if no frames
        
        Serial.write(13); //no waiting frames
        break;
    case 'A': //LAWICEL - poll for all waiting frames - CR if no frames
       
        if (SysSettings.lawicelPollCounter == 0) Serial.write(13);
        break;
    case 'F': //LAWICEL - read status bits
        Serial.print("F00"); //bit 0 = RX Fifo Full, 1 = TX Fifo Full, 2 = Error warning, 3 = Data overrun, 5= Error passive, 6 = Arb. Lost, 7 = Bus Error
        Serial.write(13);
        break;
    case 'V': //LAWICEL - get version number
        Serial.print("V1013\n");
        SysSettings.lawicelMode = true;
        break;
    case 'N': //LAWICEL - get serial number
        Serial.print("ESP32RET\n");
        SysSettings.lawicelMode = true;
        break;
    case 'x':
        SysSettings.lawicellExtendedMode = !SysSettings.lawicellExtendedMode;
        if (SysSettings.lawicellExtendedMode) {
            Serial.print("V2\n");
        }
        else {
            Serial.print("LAWICEL\n");
        }            
        break;
    case 'B': //LAWICEL V2 - Output list of supported buses
        if (SysSettings.lawicellExtendedMode) {
            for (int i = 0; i < NUM_BUSES; i++) {
                printBusName(i);
                Serial.print("\n");
            }
        }
        break;
    case 'X':
        if (SysSettings.lawicellExtendedMode) {
        }
        break;        
    }
}

void LAWICELHandler::handleLongCmd(char *buffer)
{
    char buff[80];
    int val;
    
    tokenizeCmdString(buffer);

    switch (buffer[0]) {
    
    case 'S': 
        if (!SysSettings.lawicellExtendedMode) {
            //setup canbus baud via predefined speeds
           
        }
        else { //LAWICEL V2 - Send packet out of specified bus - S <Bus> <ID> <Data0> <Data1> <...>
            uint8_t bytes[8];
            uint32_t id;
            int numBytes = 0;
            id = strtol(tokens[2], nullptr, 16);
            for (int b = 0; b < 8; b++) {
                if (tokens[3 + b][0] != 0) {
                    bytes[b] = strtol(tokens[3 + b], nullptr, 16);
                    numBytes++;
                }
                else break; //break for loop because we're obviously done.
            }
            
        }
    case 's': //setup canbus baud via register writes (we can't really do that...)
        //settings.CAN0Speed = 250000;
        break;
    case 'r': //send a standard RTR frame (don't really... that's so deprecated its not even funny)
        break;
    case 'R': 
        if (SysSettings.lawicellExtendedMode) { //Lawicel V2 - Set that we want to receive traffic from the given bus - R <BUSID>
            if (!strcasecmp(tokens[1], "CAN0")) SysSettings.lawicelBusReception[0] = true;
            if (!strcasecmp(tokens[1], "CAN1")) SysSettings.lawicelBusReception[1] = true;
        }
        else { //Lawicel V1 - send extended RTR frame (NO! DON'T DO IT!)
        }
        break;
    case 'X': //Set autopoll off/on
        if (buffer[1] == '1') SysSettings.lawicelAutoPoll = true;
        else SysSettings.lawicelAutoPoll = false;
        break;
    case 'W': //Dual or single filter mode
        break; //don't actually support this mode
    case 'm': //set acceptance mask - these things seem to be odd and aren't actually implemented yet
    case 'M': 
        if (SysSettings.lawicellExtendedMode) { //Lawicel V2 - Set filter mask - M <busid> <Mask> <FilterID> <Ext?>
            int mask = strtol(tokens[2], nullptr, 16);
            int filt = strtol(tokens[3], nullptr, 16);


        }
        else { //Lawicel V1 - set acceptance code
        }        
        break;
    case 'H':
        if (SysSettings.lawicellExtendedMode) { //Lawicel V2 - Halt reception of traffic from given bus - H <busid>
            if (!strcasecmp(tokens[1], "CAN0")) SysSettings.lawicelBusReception[0] = false;
            if (!strcasecmp(tokens[1], "CAN1")) SysSettings.lawicelBusReception[1] = false;
        } 
        break;        
    case 'U': //set uart speed. We just ignore this. You can't set a baud rate on a USB CDC port
        break; //also no action here
    case 'Z': //Turn timestamp off/on
        if (buffer[1] == '1') SysSettings.lawicelTimestamping = true;
        else SysSettings.lawicelTimestamping =  false;
        break;
    case 'Q': //turn auto start up on/off - probably don't need to actually implement this at the moment.
        break; //no action yet or maybe ever
    case 'C': //Lawicel V2 - configure one of the buses - C <busid> <speed> <any additional needed params> 
        if (SysSettings.lawicellExtendedMode) {
            //at least two parameters separated by spaces. First BUS ID (CAN0, CAN1, SWCAN, etc) then speed (or more params separated by #'s)
            int speed = atoi(tokens[2]);
            if (!strcasecmp(tokens[1], "CAN0")) {
    
            }
            if (!strcasecmp(tokens[1], "CAN1")) {
    
            }            
        }
        break;
    }
    Serial.write(13);
}

//Tokenize cmdBuffer on space boundaries - up to 10 tokens supported
void LAWICELHandler::tokenizeCmdString(char *buff) {
   int idx = 0;
   char *tok;
   
   for (int i = 0; i < 13; i++) tokens[i][0] = 0;
   
   tok = strtok(buff, " ");
   if (tok != nullptr) strcpy(tokens[idx], tok);
       else tokens[idx][0] = 0;
   while (tokens[idx] != nullptr && idx < 13) {
       idx++;
       tok = strtok(nullptr, " ");
       if (tok != nullptr) strcpy(tokens[idx], tok);
            else tokens[idx][0] = 0;
   }
}

void LAWICELHandler::uppercaseToken(char *token) {
    int idx = 0;
    while (token[idx] != 0 && idx < 9) {
        token[idx] = toupper(token[idx]);
        idx++;
    }
    token[idx] = 0;
}

void LAWICELHandler::printBusName(int bus) {
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