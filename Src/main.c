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
uint32_t BAUDRATE_LIN = 9600UL;
uint8_t count_baud_bytes = 0;
bool waitLinSlave = FALSE;
enum avCommands parsedCommand = none_command;
lin lin_received;
lin lin_transmit;
lin lin_slave_transmit;
uint32_t SysCounter = 0;
enum CRC_Type CRC_parse;
enum Filtering Filtering_parse = Show_invalid;
enum Slave_type Slave_parse = undef;

volatile uint32_t *infoPage = (uint32_t *)0x0801FC00;

/*******************************************************************************/
/*******************************************************************************/
// For usbd implementation
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
		.class_data_handler = cdc_acm_data_handler};

/*******************************************************************************/
static inline void SysInit(void);
static inline void GetBackup(enum CRC_Type *crc, enum Filtering *filt, uint32_t *baud, volatile uint32_t *pInfo, bool direction);
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
int main()
{
	char msgInf[] = "****, Enhanced:*,show:* \n\r";
	
	lin_transmit.state = wait_pid;
	lin_received.state = wait_break;
	lin_slave_transmit.state = wait_pid;
	SysInit();
	GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 0);
#ifndef USB_VCP
	print("LIN adapter ver. 1.0 2022-02-21\n\r");
	nvic_irq_enable(USART0_IRQn, 2, 1); // For UART0_PC
#else
	nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
	nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 1);
#endif
	nvic_irq_enable(USART1_IRQn, 2, 2);	   // For LIN UART IRQ
	nvic_irq_enable(TIMER0_UP_IRQn, 3, 3); // For timming definition

	for (;;)
	{
		// This part of code parse inout data buff
		/*******************************************************************************/
		// USBD chech buffer
		if (USBD_CONFIGURED == usb_device_dev.status)
		{
			if (1 == packet_receive)
			{
				cdc_acm_data_receive(&usb_device_dev);
			}
			else
			{
				if (0 != receive_length)
				{
					// cdc_acm_data_send(&usb_device_dev, receive_length);
					for (uint8_t i = 0; i < receive_length; ++i)
					{
						Push(&RS232_RX, usb_data_buffer[i]);
					}
					receive_length = 0;
				}
			}
		}
		/*******************************************************************************/
		/*******************************************************************************/
		// Parse RS232 fifo
		if (GetSize(&RS232_RX) != 0)		   // Check data from PC
		{	
			if (parsedCommand == none_command) // Receive command frame
			{
				parsedCommand = GetCommand(Pull(&RS232_RX));
				if (parsedCommand == none_command)
				{
					print("Undefined commmand\n\r");
				}
			}
			else // Receive command argument
			{
				switch (parsedCommand)
				{
					/*******************************************************************************/
				case setBaud:
					if (!receive_baudval(&BAUDRATE_LIN, &count_baud_bytes, Pull(&RS232_RX)))
					{
						parsedCommand = setBaud;
					}
					else
					{
						USART_BAUD(USART_LIN) = BAUDRATE_LIN;
						usart_baudrate_set(USART_LIN, BAUDRATE_LIN);
						print("BAUD set\n\r");
						GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 1);
						parsedCommand = none_command;
					}
					break;
					/*******************************************************************************/
				case setCRC:
					if (Pull(&RS232_RX) == 0)
					{
						CRC_parse = Classic;
						print("Classic\n\r");
					}
					else
					{
						CRC_parse = Enhanced;
						print("Enhanced\n\r");
					}
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 1);
					parsedCommand = none_command;
					break;
					/*******************************************************************************/
				case setFilter:
					if(Pull(&RS232_RX) == 1) {
						Filtering_parse = Show_invalid;
						print("Show invalid\n\r");
					}
					else{
						Filtering_parse = Hide_invalid;
						print("Hide invalid\n\r");
					}
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, infoPage, 1);
					parsedCommand = none_command;
					break;
					/*******************************************************************************/
				case getInfo: 
					Pull(&RS232_RX);
					/*******************************************************************************/
					print_num(BAUDRATE_LIN, msgInf);
					/***************************************************/
					if (CRC_parse == Classic)
					{
						msgInf[15] = '0';
					}
					else
					{
						msgInf[15] = '1';
					}
					/****************************************************/
					if (Filtering_parse == Show_invalid)
					{
						msgInf[22] = '1';
					}
					else if (Filtering_parse == Hide_invalid)
					{
						msgInf[22] = '0';
					}
					print(msgInf);
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
							if (Slave_parse == right_now)
							{
								LinDataFrameSend(&lin_slave_transmit);
								LinClear(&lin_slave_transmit);
								Slave_parse = undef;
								parsedCommand = none_command;
							}
							else
							{
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
// This part of code i use for data transmit at IRQ TXE, IRQ disable in IRQ handler
#ifndef USB_VCP
		if (GetSize(&RS232_TX) != 0)
		{
			// Enable IRQ
			usart_interrupt_enable(USART0, USART_INT_TBE);
		}
#else 
		 if (0 != GetSize(&RS232_TX)){
				uint8_t size = GetSize(&RS232_TX);
			 for(uint8_t i  = 0; i < size; ++i){
				 usb_data_buffer[i] = Pull(&RS232_TX);
			 }
			 cdc_acm_data_send(&usb_device_dev, size);
		 }
#endif
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
				// Get out 0x00 + PID + Data frame + CRC (valid)
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
					// Remove invalid packet
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
#ifndef USB_VCP
	USART0_Init();
#else
	usbd_core_init(&usb_device_dev);
#endif
	USART1_Init();
	TIM0_Init();
}

// This function for backup|read settings from flash memory
// Direction: 0 - read, 1 - write
static inline void GetBackup(enum CRC_Type *crc, enum Filtering *filt, uint32_t *baud, volatile uint32_t *pInfo, bool direction)
{
	if (direction)
	{ // Write info
		fmc_unlock();
		fmc_page_erase((uint32_t)pInfo);
		fmc_word_program((uint32_t)pInfo, *crc);
		fmc_word_program((uint32_t)pInfo + sizeof(uint32_t), *filt);
		fmc_word_program((uint32_t)pInfo + 2 * sizeof(uint32_t), *baud);
		fmc_lock();
	}
	else
	{
		*crc = (enum CRC_Type)pInfo[0];
		*filt = (enum Filtering)pInfo[1];
		*baud = (uint32_t)pInfo[2];
	}
}
