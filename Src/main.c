#include "main.h"
#include "lin.h"
#include "prot.h"
/*******************************************************************************/
/*******************************************************************************/
// User global variables declarations
struct fifo LIN_RX;
struct fifo LIN_TX;
struct fifo RS232_RX;
struct fifo RS232_TX;
uint16_t BAUDRATE_LIN = 9600UL;
bool waitLinSlave = FALSE;
enum avCommands parsedCommand = none_command;
lin lin_slave;
lin lin_received;
lin lin_transmit;
uint32_t SysCounter = 0;
enum CRC_Type CRC_parse;
enum Filtering Filtering_parse;
enum Slave_type Slave_parse;
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
static inline void SysInit(void);
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
int main()
{
	lin_transmit.state = wait_pid;
	lin_received.state = wait_break;
	SysInit();
	print("LIN adapter ver. 1.0 2022-02-21\n\r");
	nvic_irq_enable(USART0_IRQn, 2, 1); // For UART0_PC
	nvic_irq_enable(USART1_IRQn, 1, 1); // For J1708 UART IRQ
	// nvic_irq_enable(TIMER0_UP_IRQn, 2, 2); // For timming definition
	for (;;)
	{
		// This part of code parse inout data buff
		/*******************************************************************************/
		/*******************************************************************************/
		/*******************************************************************************/
		// Parse RS232 fifo
		if (GetSize(&RS232_RX) != 0)
		{ // All fields for lin packet recieved
			if (parsedCommand == none_command)
			{
				parsedCommand = GetCommand(Pull(&RS232_RX));
			}
			else
			{
				switch (parsedCommand)
				{
					/*******************************************************************************/
				case setBaud:
					BAUDRATE_LIN = Pull(&RS232_RX) * 100;
					print("Set baud\n\r");
					break;
					/*******************************************************************************/
				case setCRC:
					if (CRC_parse == Enhanced)
					{
						CRC_parse = Classic;
						print("Classic CRC selected\n\r");
					}
					else
					{
						CRC_parse = Enhanced;
						print("Enhanced CRC selected\n\r");
					}
					break;
					/*******************************************************************************/
				case setFilter:
					if (Filtering_parse == Show_invalid)
					{
						Filtering_parse = Hide_invalid;
						print("Packets with invalid CRC will be hide\n\r");
					}
					else
					{
						Filtering_parse = Show_invalid;
						print("Packets with invalid CRC will be show\n\r");
					}
					break;
					/*******************************************************************************/
				case getInfo:
					/*******************************************************************************/
					if (BAUDRATE_LIN == 9600U)
					{
						print("BAUDRATE is 9600\n\r");
					}
					else if (BAUDRATE_LIN == 19200U)
					{
						print("BAUDRATE_LIN is 19200\n\r");
					}
					else
					{
						print("Undefined BAUDRATE\n\r");
					}
					/*******************************************************************************/
					if (Filtering_parse == Show_invalid)
					{
						print("Packets with invalid CRC don't be hide \n\r");
					}
					else if (Filtering_parse == Hide_invalid)
					{
						print("Packets with invalid CRC be hide \n\r");
					}
					else
					{
						print("Mode of filtering packets don't be selected\n\r");
					}
					break;
					/*******************************************************************************/
				case sendSlave:
					if (Slave_parse == undef)
					{
						if (Pull(&RS232_RX) == 0x01)
						{
							Slave_parse = PID_compare;
						}
						else
						{
							Slave_parse = right_now;
						}
					}
					else
					{
						if (GetLinPacket(Pull(&RS232_RX), &lin_transmit))
						{
							// TODO: add send slave
						}
					}
					break;
					/*******************************************************************************/
				case sendMaster:
					if (GetLinPacket(Pull(&RS232_RX), &lin_transmit))
					{
						// TODO: Add lin send
						parsedCommand = none_command;
					}
					break;
					/*******************************************************************************/
				case none_command:
					print("Can't be recognized. Try again\n\r");
					break;
				}
			}
		}
		/*******************************************************************************/
		/*******************************************************************************/
		/*******************************************************************************/
		if (GetSize(&RS232_TX) != 0)
		{
			// Enable IRQ
			usart_interrupt_enable(USART0, USART_INT_TBE);
		}
		/*******************************************************************************/
		/*******************************************************************************/
		/*******************************************************************************/
		if (GetSize(&LIN_RX) != 0)
		{
		}
		/*******************************************************************************/
		/*******************************************************************************/
		/*******************************************************************************/
		if (GetSize(&LIN_TX) != 0)
		{
		}
		/*******************************************************************************/
		/*******************************************************************************/
		/*******************************************************************************/
		if (lin_received.state == completed)
		{ // Packet from LIN bus recognized, need to send to RS232

			if (lin_received.crc == lin_received.rcrc)
			{ // CRC is correctly
				Push(&RS232_TX, lin_received.PID);
				for (uint8_t i = 0; i < lin_received.size; ++i)
				{
					Push(&RS232_TX, lin_received.data[i]);
				}
				Push(&RS232_TX, lin_received.crc);
				lin_received.state = wait_break;
			}
			else
			{ // Invalid CRC
				if (Filtering_parse == Show_invalid)
				{
					Push(&RS232_TX, 0x00);
					Push(&RS232_TX, lin_received.PID);
					for (uint8_t i = 0; i < lin_received.size; ++i)
					{
						Push(&RS232_TX, lin_received.data[i]);
					}
					Push(&RS232_TX, lin_received.crc);
					lin_received.state = wait_break;
				}
				else
				{
					lin_received.state = wait_break;
				}
			}
		}
		/*******************************************************************************/
		/*******************************************************************************/
		/*******************************************************************************/
	}
}
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
static inline void SysInit(void)
{
	CLK_Init();
	GPIO_Init();
	USART0_Init();
	USART1_Init();
	TIM0_Init();
}
