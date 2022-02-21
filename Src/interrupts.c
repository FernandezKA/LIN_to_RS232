#include "interrupts.h"
#include "lin.h"
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
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
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
// This irq handler for lin bus
void USART1_IRQHandler(void)
{
	if (usart_flag_get(USART0, USART_FLAG_LBDF))
	{ // Break field detection
		usart_flag_clear(USART0, USART_FLAG_LBDF);
		lin_received.state = wait_synch;
	}
	else if (usart_flag_get(USART0, USART_FLAG_RBNE))
	{ // data fields detection
		switch (lin_received.state)
		{
		case wait_break:
			__NOP();
			break;

		case wait_synch:
			if (usart_data_receive(USART0) == 0x55U)
			{
				lin_received.state = wait_pid;
			}
			else
			{
				lin_received.state = wait_break;
			}
			break;

		case wait_pid:
			lin_received.PID = (uint8_t)usart_data_receive(USART0);
			lin_received.size = GetLinSize(&lin_received);
			lin_received.state = wait_data;
			break;

		case wait_data:
			if (lin_received.countData < lin_received.size - 1)
			{
				lin_received.data[lin_received.countData++] = (uint8_t)usart_data_receive(USART0);
			}
			else
			{
				lin_received.data[lin_received.countData++] = (uint8_t)usart_data_receive(USART0);
				lin_received.state = wait_crc;
			}
			break;

		case wait_crc:
			lin_received.rcrc = (uint8_t)usart_data_receive(USART0);
			lin_received.crc = GetCRC(&lin_received, &CRC_parse);
			lin_received.state = completed;
			break;

		case completed:
			// This case not used
			__NOP();
			break;
		}
	}
	else
	{ // Undefined behavioral
	}
}
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
// At now unused, will be used for indicate
void TIMER0_UP_IRQHandler(void)
{
	SysCounter++;
	if (SysCounter % 2000000 == 0)
	{
		GPIO_OCTL(GPIOC) ^= (1 << 13);
	}
}
