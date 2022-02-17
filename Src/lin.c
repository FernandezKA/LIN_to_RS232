#include "main.c"
#include "lin.h"

bool GetLinPacket(uint8_t data, lin* packet){
	bool status = false;
	
	switch(packet->state){
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
