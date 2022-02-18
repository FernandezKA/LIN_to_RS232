#include "main.h"
#include "lin.h"
//User global variables declarations
struct fifo LIN_RX;
struct fifo LIN_TX;
struct fifo RS232_RX;
struct fifo RS232_TX;

bool waitLinSlave = FALSE;
lin lin_slave;

static inline void SysInit(void);

int main() {
SysInit();
	for(;;){
//This part of code parse inout data buff
		//Parse lin FIFO
	if(GetSize(&LIN_RX) != 0){
		switch(lin_received.state){
			case wait_break:
				 __NOP();
			break;
			
			case wait_pid:
				 lin_received.PID = (uint8_t) (Pull(&LIN_RX)&0x3F);
				if(lin_received.PID < 0x1F){
				 lin_received.size = 2;
				}
				else if(lin_received.PID > 0x1F && lin_received.PID < 0x2F){
				 lin_received.size = 4;
				}
				else{
					lin_received.size = 8;
				}
				lin_received.state = wait_data;
			break;
			
			case wait_data:
				 if(lin_received.countData < lin_received.size){
						lin_received.data[lin_received.countData++] = Pull(&LIN_RX);
				 }
				 else{
						lin_received.data[lin_received.countData++] = Pull(&LIN_RX);
						lin_received.state = wait_crc;
				 }
			break;
			
			case wait_crc:
					lin_received.crc = 
			break;
			
			case completed:
				
			break;
		}
	}
	
	if(GetSize(&LIN_TX) != 0){
		//Enable IRQ
		
	}
		//Parse RS232 fifo
	if(GetSize(&RS232_RX) != 0){     //All fields for lin packet recieved
//		if(GetLinPacket(Pull(&RS232_RX), &lin_transmit)){
//			
//		}
//		else{   //Wait new field for LIN pa
//			
//		}
	}
		
	if(GetSize(&RS232_TX) != 0){
		//Enable IRQ
	}	
	}
}

static inline void SysInit(void){
	CLK_Init();
	GPIO_Init();
	USART0_Init();
	USART1_Init();
	TIM0_Init();
}

