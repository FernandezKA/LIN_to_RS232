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

void print(char *pMsg);

void print_num(uint32_t num, char* msg);

bool receive_baudval(uint32_t *baud, uint8_t *countBytes, uint8_t currByte);
