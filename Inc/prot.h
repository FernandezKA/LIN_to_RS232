#pragma once

#include "main.h"

enum avCommands
{
    setBaud,
    setCRC,
    setFilter,
    getInfo,
    sendSlave,
    sendMaster,
    none_command
};

extern enum avCommands parsedCommand;

enum avCommands GetCommand(uint8_t currByte);

void print(char* pMsg);

