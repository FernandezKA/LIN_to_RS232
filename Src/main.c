#include "main.h"
#include "lin.h"
#include "prot.h"
#include "cdc_core.h"
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
lin lin_received;
lin lin_transmit;
lin lin_slave_transmit;
uint32_t SysCounter = 0;
enum CRC_Type CRC_parse;
enum Filtering Filtering_parse = Show_invalid;
enum Slave_type Slave_parse = undef;

volatile uint32_t*  infoPage = (uint32_t*) 0x0801FC00; 

/*******************************************************************************/
/*******************************************************************************/
//For usbd implementation 
extern uint8_t packet_sent, packet_receive;
extern uint32_t receive_length;
extern uint8_t usb_data_buffer[CDC_ACM_DATA_PACKET_SIZE];

usbd_core_handle_struct usb_device_dev =
	{
		.dev_desc = (uint8_t *)&device_descriptor,
		.config_desc = (uint8_t *)&configuration_descriptor,
		.strings = usbd_strings,
		.class_init = cdc_acm_init,
		.class_deinit = cdc_acm_deinit,
		.class_req_handler = cdc_acm_req_handler,
		.class_data_handler = cdc_acm_data_handler
	};

/*******************************************************************************/
static inline void SysInit(void);
static inline void GetBackup(enum CRC_Type* crc, enum Filtering* filt, uint16_t* baud, volatile uint32_t* pInfo, bool direction);
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
int main()
{
	lin_transmit.state = wait_pid;
	lin_received.state = wait_break;
	lin_slave_transmit.state = wait_pid;
	SysInit();
	GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 0);
	print("LIN adapter ver. 1.0 2022-02-21\n\r");
	nvic_irq_enable(USART0_IRQn, 2, 1); // For UART0_PC
	nvic_irq_enable(USART1_IRQn, 1, 1); // For LIN UART IRQ
	// nvic_irq_enable(TIMER0_UP_IRQn, 2, 2); // For timming definition
	
	for (;;)
	{
		// This part of code parse inout data buff
		/*******************************************************************************/
		/*******************************************************************************/
		/*******************************************************************************/
		// Parse RS232 fifo
		if (GetSize(&RS232_RX) != 0)//Check data from PC
		{ // All fields for lin packet recieved
			if (parsedCommand == none_command)//Receive command frame
			{
				parsedCommand = GetCommand(Pull(&RS232_RX));
				if(parsedCommand == none_command){
					 print("Undefined commmand\n\r");
				}
				else{
					 print("Enter argument\n\r");
				}
			}
			else//Receive command argument
			{
				switch (parsedCommand)
				{
					/*******************************************************************************/
				case setBaud:
					BAUDRATE_LIN = Pull(&RS232_RX) * 100;
					USART_BAUD(USART_LIN) = BAUDRATE_LIN;
					print("BAUDRATE is set\n\r");
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 1);
					parsedCommand = none_command;
					break;
					/*******************************************************************************/
				case setCRC:
					if (Pull(&RS232_RX) == 0)
					{
						CRC_parse = Classic;
						print("Classic CRC selected\n\r");
					}
					else
					{
						CRC_parse = Enhanced;
						print("Enhanced CRC selected\n\r");
					}
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 1);
					parsedCommand = none_command;
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
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 1);
					parsedCommand = none_command;
					break;
					/*******************************************************************************/
				case getInfo:
					Pull(&RS232_RX);
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
					/***************************************************/
					if(CRC_parse == Classic){
						print("Classic CRC calculate\n\r");
					}
					else{
						print("Enhanced CRC calculate\n\r");
					}
					/****************************************************/
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
					parsedCommand = none_command;
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
						if (GetLinPacket(Pull(&RS232_RX), &lin_slave_transmit))
						{
							if(Slave_parse == right_now){
								LinDataFrameSend(&lin_slave_transmit);
								LinClear(&lin_slave_transmit);
								Slave_parse = undef;
								parsedCommand = none_command;
							}
							else{
							lin_slave_transmit.state = completed;
							parsedCommand = none_command;
							Slave_parse = undef;
							}
						}
					}
					break;
					/*******************************************************************************/
				case sendMaster:
					if (GetLinPacket(Pull(&RS232_RX), &lin_transmit))
					{
						print("Packet received from RS232\n\r");
						LinSend(&lin_transmit);
						LinClear(&lin_transmit);
						parsedCommand = none_command;
						// TODO: Add lin send
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
			//Get out 0x00 + PID + Data frame + CRC (valid)
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
					//Remove invalid packet
					lin_received.state = wait_break;
				}
			}
		}
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

//This function for backup|read settings from flash memory
//Direction: 0 - read, 1 - write
static inline void GetBackup(enum CRC_Type* crc, enum Filtering* filt, uint16_t* baud, volatile uint32_t* pInfo, bool direction){
	if(direction){//Write info
	fmc_unlock();
	fmc_page_erase((uint32_t) pInfo);
	fmc_word_program((uint32_t) pInfo ,*crc);
	fmc_word_program((uint32_t) pInfo + sizeof(uint32_t) ,*filt);
	fmc_word_program((uint32_t) pInfo + 2 * sizeof(uint32_t) ,*baud);
	fmc_lock();
	}
	else{
	*crc = (enum CRC_Type) pInfo[0];
	*filt = (enum Filtering) pInfo[1];
	*baud = (uint16_t) pInfo[2];
	}
}

