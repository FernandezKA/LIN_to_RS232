#include "interrupts.h"
#include "lin.h"
// This IRQ Handler for USART PC
void USART0_IRQHandler(void)
{
	if (usart_flag_get(USART0, USART_FLAG_RBNE))
	{
		Push(&RS232_RX, (uint8_t)usart_data_receive(USART0));
	}
	else if (usart_flag_get(USART0, USART_FLAG_TBE))
	{
		if (GetSize(&RS232_TX) != 0)
		{
			usart_data_transmit(USART0, Pull(&RS232_TX));
		}
		else
		{
			usart_interrupt_disable(USART0, USART_INT_TBE);
		}
	}
	else
	{
		USART_STAT(USART0) = 0;
	}
}
// This irq handler for lin bus
void USART1_IRQHandler(void)
{
	if (usart_flag_get(USART0, USART_FLAG_LBDF))
	{ // Break field detection
		usart_flag_clear(USART0, USART_FLAG_LBDF);
		lin_received.state = wait_pid;
	}
	else if (usart_flag_get(USART0, USART_FLAG_RBNE))
	{ // data fields detection
	}
	else
	{ // Undefined behavioral
	}
}
// At now unused, will be used for indicate
void TIMER0_UP_IRQHandler(void)
{
	SysCounter++;
	if (SysCounter % 2000000 == 0)
	{
		GPIO_OCTL(GPIOC) ^= (1 << 13);
	}
}
