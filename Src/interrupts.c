#include "interrupts.h"
#include "lin.h"
#include "main.h"
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
		if((USART_STAT(USART_LIN) & USART_STAT_ORERR) == USART_STAT_ORERR){
			 lin_received.state = wait_break;
			usart_data_receive(USART_LIN);
		}
		else{
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
			lin_received.size = lin_size_get(&lin_received);
			lin_received.state = wait_data;
			//for send data slave packet at comare pid
			if (lin_slave_transmit_compare.state == completed && Slave_parse == PID_compare)
			{
				if (lin_received.PID == lin_slave_transmit_compare.PID)
				{
					// Get send our slave packet
					if (MUTE_MODE == 0xFFFFFFFFU)
					{
						lin_repeat_slave(&lin_slave_transmit_compare);
					}
					lin_send_data_frame(&lin_slave_transmit_compare);
					lin_struct_clear(&lin_slave_transmit_compare);
					lin_struct_clear(&lin_received);
					lin_slave_transmit_compare.state = wait_pid;
					lin_received.state = wait_break;
				}
			}
			break;

		case wait_data:
			if (lin_received.countData < lin_received.size - 1U)
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
			lin_received.crc = lin_crc_get(&lin_received, &CRC_parse);
			lin_received.state = completed;
			break;

		case completed:
			// This case not used
			__NOP();
			break;
		}
	}
	}
}
// used for indicate on PC13
void TIMER0_UP_IRQHandler(void)
{
	TIMER_INTF(TIMER0) &= ~TIMER_INTF_UPIF;
	GPIO_OCTL(GPIOC) ^= (1U << 13);
}
//IRQ handler for usbd cdc
void USBD_LP_CAN0_RX0_IRQHandler(void)
{
	usbd_isr();
}
