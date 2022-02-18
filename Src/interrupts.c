#include "interrupts.h"
#include "lin.h"
void USART0_IRQHandler(void){
__NOP();
}

void USART1_IRQHandler(void){
	if(usart_flag_get(USART0, USART_FLAG_LBDF)){ //Break field detection
		usart_flag_clear(USART0, USART_FLAG_LBDF);
		 lin_received.state = wait_pid;
	}
	else if (usart_flag_get(USART0, USART_FLAG_RBNE)){  //data fields detection
		
	}
	else{ //Undefined behavioral
	
	}
}

void TIMER0_UP_IRQHandler(void){
__NOP();
}
