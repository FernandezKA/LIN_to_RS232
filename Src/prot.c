#include "prot.h"
#include "main.h"
enum avCommands GetCommand(uint8_t currByte)
{
    enum avCommands currCommand = none_command;
    switch (currByte)
    {
//    case 'b':
//        currCommand = setBaud;
//        break;

//    case 'B':
//        currCommand = setBaud;
//        break;

//    case 'e':
//        currCommand = setCRC;
//        break;

//    case 'E':
//        currCommand = setCRC;
//        break;

//    case 'c':
//        currCommand = setFilter;
//        break;

//    case 'C':
//        currCommand = setFilter;
//        break;

//    case 'i':
//        currCommand = getInfo;
//        break;

//    case 'I':
//        currCommand = getInfo;
//        break;

//    case 's':
//        currCommand = sendSlave;
//        break;

//    case 'S':
//        currCommand = sendSlave;
//        break;

//    case 'm':
//        currCommand = sendMaster;
//        break;

//    case 'M':
//        currCommand = sendMaster;
//        break;

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
void print_num(uint32_t num)
{
    usb_data_buffer[0] = ((num & 0xFF000000) >> 24);
    usb_data_buffer[1] = ((num & 0x00FF0000) >> 16);
    usb_data_buffer[2] = ((num & 0x0000FF00) >> 8);
    usb_data_buffer[3] = ((num & 0x000000FF));
#ifdef USB_VCP
    if (USBD_CONFIGURED == usb_device_dev.status)
    {
        cdc_acm_data_send(&usb_device_dev, 4);
    }
#endif
}
// This function receive baudval
bool receive_baudval(uint32_t *baud, uint8_t *countBytes, uint8_t currByte)
{
    if (countBytes == 0)
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
        *countBytes = 0;
        *baud |= currByte << ((3 - *countBytes) * 8);
        return true;
    }
}
