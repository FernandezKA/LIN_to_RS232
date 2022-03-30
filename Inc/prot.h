#pragma once

#include "main.h"

enum avCommands
{
    setBaud,
    setCRC,
    setFilter,
    getInfo,
		muteMode,
    sendSlave,
    sendMaster,
    none_command
};

extern enum avCommands parsedCommand;

enum avCommands GetCommand(uint8_t currByte);

void print(char *pMsg);

void send_array(uint8_t* ptr, uint8_t size);

bool receive_baudval(uint32_t *baud, uint8_t *countBytes, uint8_t currByte);
