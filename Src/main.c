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

