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
extern lin lin_slave_transmit_compare;
extern bool waitLinSlave;

void lin_struct_clear(lin *packet);

bool lin_receive_packet(uint8_t data, lin *packet);

void lin_send_master(lin *packet);

uint8_t lin_crc_get(lin *packet, enum CRC_Type *crc_type);

uint8_t lin_size_get(lin *packet);

void lin_send_data_frame(lin *packet_slave);

static inline void _send_one_byte_lin(uint8_t data);

void lin_repeat_slave(lin* packet);

void lin_repeat_master(lin* packet);
