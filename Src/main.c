#include "main.h"
#include "lin.h"
#include "prot.h"
// User global variables declarations
struct fifo LIN_RX;
struct fifo LIN_TX;
struct fifo RS232_RX;
struct fifo RS232_TX;

bool waitLinSlave = FALSE;
enum avCommands parsedCommand = none_command;
lin lin_slave;

lin lin_received;
lin lin_transmit;

uint32_t SysCounter = 0;

static inline void SysInit(void);

int main()
{
	lin_transmit.state = wait_pid;
	SysInit();
	nvic_irq_enable(USART0_IRQn, 2, 1); // For UART0_PC
	nvic_irq_enable(USART1_IRQn, 1, 1); // For J1708 UART IRQ
										// nvic_irq_enable(TIMER0_UP_IRQn, 2, 2); // For timming definition
	for (;;)
	{
		// This part of code parse inout data buff

		// Parse RS232 fifo
		if (GetSize(&RS232_RX) != 0)
		{ // All fields for lin packet recieved
			uint8_t currByte = Pull(&RS232_RX);
			if (parsedCommand == none_command)
			{
				parsedCommand = GetCommand(currByte);
			}
			else
			{
				switch (parsedCommand)
				{
				case setBaud:

					break;

				case setCRC:

					break;

				case setFilter:

					break;

				case getInfo:

					break;

				case sendSlave:

					break;

				case sendMaster:
					if (GetLinPacket(currByte, &lin_transmit))
					{
						// TODO: Add lin send
						parsedCommand = none_command;
					}
					else{
						GetLinPacket(currByte, &lin_transmit);
					}
					break;

				case none_command:

					break;
				}
			}
		}

		if (GetSize(&RS232_TX) != 0)
		{
			// Enable IRQ
			usart_interrupt_enable(USART0, USART_INT_TBE);
		}
	}
}

static inline void SysInit(void)
{
	CLK_Init();
	GPIO_Init();
	USART0_Init();
	USART1_Init();
	TIM0_Init();
}
