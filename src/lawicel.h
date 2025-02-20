#pragma once

class CAN_FRAME;

class LAWICELHandler
{
public:
    void handleLongCmd(char *buffer);
    void handleShortCmd(char cmd);
    

private:
    char tokens[14][10];

    void tokenizeCmdString(char *buff);
    void uppercaseToken(char *token);
    void printBusName(int bus);
    
};