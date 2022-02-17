#pragma once

#include <stdint.h>
#include <stdbool.h>

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

void LinClear(lin* packet);

bool GetLinPacket(uint8_t data, lin* packet);


