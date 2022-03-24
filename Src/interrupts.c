#include "interrupts.h"
#include "lin.h"
#include "main.h"
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
// This IRQ Handler for USART PC
void USART0_IRQHandler(void)
{
	if (usart_flag_get(USART_RS232, USART_FLAG_RBNE))
	{
		usart_flag_clear(USART_RS232, USART_FLAG_RBNE);
		Push(&RS232_RX, (uint8_t)usart_data_receive(USART_RS232));
	}
	else if (usart_flag_get(USART_RS232, USART_FLAG_TBE))
	{
		if (GetSize(&RS232_TX) != 0)
		{
			usart_data_transmit(USART_RS232, Pull(&RS232_TX));
		}
		else
		{
			usart_interrupt_disable(USART_RS232, USART_INT_TBE);
		}
	}
	else
	{
		USART_STAT(USART_RS232) = 0;
	}
}
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
// This irq handler for lin bus
void USART1_IRQHandler(void)
{
	if (usart_flag_get(USART_LIN, USART_FLAG_LBDF))
	{ // Break field detection
		usart_flag_clear(USART_LIN, USART_FLAG_LBDF);
		lin_received.state = wait_synch;
	}
	else if (usart_flag_get(USART_LIN, USART_FLAG_RBNE))
	{ // data fields detection
		switch (lin_received.state)
		{
		case wait_break:
			__NOP();
			break;

		case wait_synch:
			if (usart_data_receive(USART_LIN) == 0x55U)
			{
				lin_received.state = wait_pid;
			}
			else
			{
				lin_received.state = wait_break;
			}
			break;

		case wait_pid:
			lin_received.PID = (uint8_t)usart_data_receive(USART_LIN);
			lin_received.size = GetLinSize(&lin_received);
			if (lin_slave_transmit.state == completed && Slave_parse == PID_compare)
			{
				if (lin_received.PID == lin_slave_transmit.PID)
				{
					// Get send our slave packet
					LinDataFrameSend(&lin_slave_transmit);
				}
			}
			lin_received.state = wait_data;
			break;

		case wait_data:
			if (lin_received.countData < lin_received.size - 1)
			{
				lin_received.data[lin_received.countData++] = (uint8_t)usart_data_receive(USART_LIN);
			}
			else
			{
				lin_received.data[lin_received.countData++] = (uint8_t)usart_data_receive(USART_LIN);
				lin_received.state = wait_crc;
			}
			break;

		case wait_crc:
			lin_received.rcrc = (uint8_t)usart_data_receive(USART_LIN);
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
	TIMER_INTF(TIMER0)&=~TIMER_INTF_UPIF;
	GPIO_OCTL(GPIOC) ^= (1 << 13);
}
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
void USBD_LP_CAN0_RX0_IRQHandler(void)
{
	usbd_isr();
}
