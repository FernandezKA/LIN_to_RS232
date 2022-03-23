#include "prot.h"
enum avCommands GetCommand(uint8_t currByte)
{
	enum avCommands currCommand = none_command;
    switch (currByte)
    {
    case 'b':
        currCommand = setBaud;
        break;

    case 'B':
        currCommand = setBaud;
        break;

    case 'e':
        currCommand = setCRC;
        break;

    case 'E':
        currCommand = setCRC;
        break;

    case 'c':
        currCommand = setFilter;
        break;

    case 'C':
        currCommand = setFilter;
        break;

    case 'i':
        currCommand = getInfo;
        break;

    case 'I':
        currCommand = getInfo;
        break;

    case 's':
        currCommand = sendSlave;
        break;

    case 'S':
        currCommand = sendSlave;
        break;

    case 'm':
        currCommand = sendMaster;
        break;

    case 'M':
        currCommand = sendMaster;
        break;
    }
		return currCommand;
}

void print(char* pMsg){
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
		while((USART_STAT(USART0)&USART_STAT_TBE) != USART_STAT_TBE){__NOP();}
		usart_data_transmit(USART0, currChar);
	}
}

