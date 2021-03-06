#include "prot.h"
#include "main.h"
//This function parse received data to enumerate of commnd 
enum avCommands GetCommand(uint8_t currByte)
{
	enum avCommands currCommand = none_command;
	switch (currByte)
	{
	case 0x01:
		currCommand = getInfo;
		break;

	case 0x02:
		currCommand = setBaud;
		break;

	case 0x03:
		currCommand = setCRC;
		break;

	case 0x04:
		currCommand = setFilter;
		break;

	case 0x05:
		currCommand = muteMode;
		break;

	case 0x06:
		currCommand = sendSlave;
		break;

	case 0x07:
		currCommand = sendMaster;
		break;
	}
	return currCommand;
}
//This function send text to USBD VCP
void print(char *pMsg)
{
	uint8_t countSend = 0;
	char lastChar = 0;
	char currChar = 0;
	bool isEnd = FALSE;
	while (!isEnd && countSend != 0xFF)
	{
		lastChar = currChar;
		currChar = pMsg[countSend++];
		if ((lastChar == 0x0A) && (currChar == 0x0D))
		{
			isEnd = TRUE;
		}
		else if ((lastChar == 0x0D) && (currChar == 0x0A))
		{
			isEnd = TRUE;
		}
		usb_data_buffer[countSend - 1] = currChar;
#ifndef USB_VCP
		while ((USART_STAT(USART_PC) & USART_STAT_TBE) != USART_STAT_TBE)
		{
			__NOP();
		}
		usart_data_transmit(USART_PC, currChar);
#endif
	}
#ifdef USB_VCP
	if (USBD_CONFIGURED == usb_device_dev.status)
	{
		while (1 != packet_sent)
		{
			__NOP();
		}
		cdc_acm_data_send(&usb_device_dev, countSend);
	}
#endif
}
//This function send array of data to USBD VCP
void send_array(uint8_t *ptr, uint8_t size)
{
	for (uint8_t i = 0; i < size; ++i)
	{
		usb_data_buffer[i] = ptr[i];
	}
	if (USBD_CONFIGURED == usb_device_dev.status)
	{
		while (1 != packet_sent)
		{
			__NOP();
		}
		cdc_acm_data_send(&usb_device_dev, size);
	}
}
// This function converts uint32_t to 4 * uint8_t for data transmission
void print_num(uint32_t num, char *msg)
{
	msg[0] = ((num & 0xFF000000) >> 24);
	msg[1] = ((num & 0x00FF0000) >> 16);
	msg[2] = ((num & 0x0000FF00) >> 8);
	msg[3] = ((num & 0x000000FF));
}
// This function receive baudval, formed 32 - bits field on 4 * 8 bits data
bool receive_baudval(uint32_t *baud, uint8_t *countBytes, uint8_t currByte)
{
	if (*countBytes == 0)
	{
		*baud = currByte << ((3 - *countBytes) * 8);
		++(*countBytes);
		return false;
	}
	else if (*countBytes < 3)
	{
		*baud |= currByte << ((3 - *countBytes) * 8);
		++(*countBytes);
		return false;
	}
	else
	{
		*baud |= currByte << ((3 - *countBytes) * 8);
		*countBytes = 0;
		return true;
	}
}
