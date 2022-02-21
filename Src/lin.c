#include "main.h"
#include "lin.h"

// This function get lin packet from RS232 data
bool GetLinPacket(uint8_t data, lin *packet)
{
	bool status = false;

	switch (packet->state)
	{
	case wait_pid:
		packet->PID = data;
		packet->size = GetLinSize(packet);
		packet->state = wait_data;
		break;

	case wait_data:
		if (packet->countData < packet->size - 1)
		{
			packet->data[packet->countData++] = data;
		}
		else
		{
			packet->data[packet->countData++] = data;
			packet->crc = GetCRC(packet, 1);
			packet->state = wait_pid;
			status = TRUE;
		}
		break;
	}
	return status;
}

// This function clear lin structure
void LinClear(lin *packet)
{
	packet->PID = 0;
	packet->size = 0;
	packet->crc = 0;
	packet->rcrc = 0;
	packet->countData = 0;
}

// This function send lin packet
void LinSend(lin *packet, bool isMaster)
{
	if (isMaster)
	{
		while ((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE)
		{
			__NOP();
		}
		usart_data_transmit(USART_LIN, 0x55U);
		while ((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE)
		{
			__NOP();
		}
		usart_data_transmit(USART_LIN, packet->PID);
		while ((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE)
		{
			__NOP();
		}
		for (uint16_t i = 0; i < packet->size; ++i)
		{
			while ((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE)
			{
				__NOP();
			}
			usart_data_transmit(USART_LIN, packet->data[i]);
		}
		while ((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE)
		{
			__NOP();
		}
		if (packet->crc == packet->rcrc)
		{
			usart_data_transmit(USART_LIN, packet->crc);
		}
		else
		{
			usart_data_transmit(USART_LIN, packet->rcrc);
		}
	}
	else
	{
		lin_slave = *packet;
		waitLinSlave = TRUE;
	}
}

// This function return lin packet size from current PID
uint8_t GetLinSize(lin *packet)
{
	uint8_t pid = (packet->PID) & 0x3FU;
	if (pid < 0x1F)
	{
		return 2;
	}
	else if (pid > 0x1f && pid < 0x2F)
	{
		return 4;
	}
	else
	{
		return 8;
	}
}

// This function calculate CRC for lin packet
uint8_t GetCRC(lin *packet, bool isEnhanced)
{
	uint8_t sum = 0;
	if (isEnhanced)
	{
		sum = packet->PID;
	}
	else
	{
		sum = 0;
	}

	for (uint8_t i = 0; i < packet->size; ++i)
	{
		sum += packet->data[i];
	}

	return sum ^ 0xFF;
}
