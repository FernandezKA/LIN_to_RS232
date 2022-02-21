#pragma once

#include <stdint.h>
#include <stdbool.h>

enum lin_state
{
	wait_break,
	wait_synch,
	wait_pid,
	wait_data,
	wait_crc,
	completed
};

struct LIN
{
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
extern lin lin_slave_transmit;
extern bool waitLinSlave;

void LinClear(lin *packet);

bool GetLinPacket(uint8_t data, lin *packet);

void LinSend(lin *packet);

uint8_t GetCRC(lin *packet, enum CRC_Type* crc_type);

uint8_t GetLinSize(lin *packet);

void LinDataFrameSend(lin* packet_slave);

static inline void SendLIN(uint8_t data);
