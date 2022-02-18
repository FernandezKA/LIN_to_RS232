#include "main.h"
#include "lin.h"

bool GetLinPacket(uint8_t data, lin* packet){
	bool status = false;
	
	switch(packet->state){
		case wait_break:
			
		break;
		
		case wait_pid:
		packet->PID = data;
		packet->state = wait_data;
		break;
		
		case wait_data:
			if(packet->countData < packet -> size){
				 packet->data[packet->countData++] = data;
			}
			else{
				packet->data[packet->countData++] = data;
				packet->state = wait_crc;
			}
		break;
		
		case wait_crc:
			
		break;
		
		case completed:
					status = true;
		break;
	}
	return status;
}


void LinClear(lin* packet){
	packet->PID = 0;
	packet->size = 0;
	packet->crc = 0;
	packet->rcrc = 0;
	packet->countData = 0;
}

void LinSend(lin* packet, bool isMaster){
	if(isMaster){
	 while((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE) {__NOP();}
	 usart_data_transmit(USART_LIN, 0x55U);
	 while((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE) {__NOP();}
	 usart_data_transmit(USART_LIN, packet->PID);
	 while((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE) {__NOP();}
	 for(uint16_t i = 0; i  < packet->size; ++i){
	 while((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE) {__NOP();}
	 usart_data_transmit(USART_LIN, packet->data[i]);
	 }
	 while((USART_STAT(USART_LIN) & USART_STAT_TBE) == USART_STAT_TBE) {__NOP();}
	 if(packet->crc == packet -> rcrc){
		usart_data_transmit(USART_LIN, packet->crc);
	 }
	 else{
		usart_data_transmit(USART_LIN, packet->rcrc);
	 }
 }
	else{
		lin_slave = *packet;
		waitLinSlave = TRUE;
	}
}

