#include "prot.h"
#include "main.h"
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
				currCommand = sendSlave;
			break;
			
			case 0x06:
				currCommand = sendMaster;
			break;
    }
    return currCommand;
}

void print(char *pMsg)
{
uint8_t countSend = 0;
	char lastChar = 0;
	char currChar = 0;
	bool isEnd = FALSE;
	while(!isEnd && countSend != 0xFF){
		lastChar = currChar;
		currChar = pMsg[countSend++];
		if((lastChar == 0x0A) && (currChar == 0x0D)){
			 isEnd = TRUE;
		}
		else if((lastChar == 0x0D) && (currChar == 0x0A)){
			 isEnd = TRUE;
		}
		usb_data_buffer[countSend - 1] = currChar;
		#ifndef USB_VCP 
		while((USART_STAT(USART_PC)&USART_STAT_TBE) != USART_STAT_TBE){__NOP();}
		usart_data_transmit(USART_PC, currChar);
		#endif
	}
	#ifdef USB_VCP
		if (USBD_CONFIGURED == usb_device_dev.status)
		{
		cdc_acm_data_send(&usb_device_dev, countSend);
		}
	#endif
}
// This function send to vcp nums
void print_num(uint32_t num, char* msg)
{
    msg[0] = ((num & 0xFF000000) >> 24);
    msg[1] = ((num & 0x00FF0000) >> 16);
    msg[2] = ((num & 0x0000FF00) >> 8);
    msg[3] = ((num & 0x000000FF));
}
// This function receive baudval
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
