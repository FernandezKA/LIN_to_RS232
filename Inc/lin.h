#pragma once

#include <stdint.h>
#include <stdbool.h>

#define USART_RS232 USART0
#define USART_LIN USART1

enum lin_state{
	wait_pid,
	wait_data,
	wait_crc,
	completed
};

struct LIN{
	uint8_t PID;
	uint8_t data[8U];
	uint8_t crc;
	uint8_t rcrc;
	uint8_t size;
	uint8_t countData;
	enum lin_state state;
};

typedef struct LIN lin;

extern lin lin_received;
extern lin lin_transmit;
extern lin lin_slave;
extern bool waitLinSlave;

void LinClear(lin* packet);

bool GetLinPacket(uint8_t data, lin* packet);

void LinSend(lin* packet, bool isMaster);

