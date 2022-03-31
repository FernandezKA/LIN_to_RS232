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
uint32_t MUTE_MODE = 0U;
uint8_t count_baud_bytes = 0U;
bool waitLinSlave = FALSE;
static volatile enum avCommands parsedCommand = none_command;
lin lin_received;
lin lin_transmit;
lin lin_slave_transmit;
uint32_t SysCounter = 0;
enum CRC_Type CRC_parse;
enum Filtering Filtering_parse = Show_invalid;
enum Slave_type Slave_parse = undef;
volatile uint32_t *infoPage = (uint32_t *)0x08007C00;

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
static inline void usbd_polling(void);
static inline void nvic_enable(void);
static inline void GetBackup(enum CRC_Type *crc, enum Filtering *filt, uint32_t *baud, uint32_t *mute,\
 volatile uint32_t *pInfo, bool direction);
/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/
int main()
{

	lin_transmit.state = wait_pid;
	lin_received.state = wait_break;
	lin_slave_transmit.state = wait_pid;
	SysInit();
	GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, &MUTE_MODE, infoPage, 0);
	nvic_enable();

	for (;;)
	{
		/*******************************************************************************/
		usbd_polling();//Check usbd for new data packets
		/*******************************************************************************/
		// Parse RS232 fifo
		if (GetSize(&RS232_RX) != 0) // Check data from PC
		{
			if (parsedCommand == none_command) // Receive command frame
			{
				parsedCommand = GetCommand(Pull(&RS232_RX));
				if (parsedCommand == none_command)
				{
					print("Undefined commmand\n\r");
					Clear(&RS232_RX);
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
						GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, &MUTE_MODE, infoPage, 1);
						Clear(&RS232_RX);
						parsedCommand = none_command;
					}
					break;
					/*******************************************************************************/
				case setCRC:
					if (Pull(&RS232_RX) == 0)
					{
						CRC_parse = Classic;
						print("Classic CRC\n\r");
					}
					else
					{
						CRC_parse = Enhanced;
						print("Enhanced CRC\n\r");
					}
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, &MUTE_MODE, infoPage, 1);
					Clear(&RS232_RX);
					parsedCommand = none_command;
					break;
					/*******************************************************************************/
				case setFilter:
					if (Pull(&RS232_RX) == 1)
					{
						Filtering_parse = Show_invalid;
						print("Show invalid packets\n\r");
					}
					else
					{
						Filtering_parse = Hide_invalid;
						print("Hide invalid packets\n\r");
					}
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, &MUTE_MODE, infoPage, 1);
					Clear(&RS232_RX);
					parsedCommand = none_command;
					break;
					/*******************************************************************************/
				case getInfo:
					// Get current configuration of device
					if (0 == Pull(&RS232_RX))
					{
						/*******************************************************************************/
						static uint8_t info[9];
						info[0] = ((BAUDRATE_LIN & 0xFF000000) >> 24);
						info[1] = ((BAUDRATE_LIN & 0x00FF0000) >> 16);
						info[2] = ((BAUDRATE_LIN & 0x0000FF00) >> 8);
						info[3] = ((BAUDRATE_LIN & 0x000000FF));
						/***************************************************/
						if (CRC_parse == Classic)
						{
							info[4] = 0x00;
						}
						else
						{
							info[4] = 0xFF;
						}
						/****************************************************/
						if (Filtering_parse == Show_invalid)
						{
							info[5] = 0xFF;
						}
						else if (Filtering_parse == Hide_invalid)
						{
							info[5] = 0x00;
						}
						/*****************************************************/
						if (MUTE_MODE == 0)
						{
							info[6] = 0xFF;
						}
						else
						{
							info[6] = 0x00;
						}
						info[7] = 0x0A;
						info[8] = 0x0D;
						send_array(info, sizeof(info));
					}
					// Get info about version of software
					else
					{
						print("LIN to USB VCP, ver. 1.0 2022-03-30\n\r");
					}
					Clear(&RS232_RX);
					parsedCommand = none_command;
					break;
					/*******************************************************************************/

				case muteMode:
					if (Pull(&RS232_RX) == 0)
					{
						print("Mute mode off\n\r");
						MUTE_MODE = 0xFFFFFFFF;
					}
					else
					{
						print("Mute mode on\n\r");
						MUTE_MODE = 0;
					}
					GetBackup(&CRC_parse, &Filtering_parse, &BAUDRATE_LIN, &MUTE_MODE, infoPage, 1);
					Clear(&RS232_RX);
					parsedCommand = none_command;
					break;

				case sendSlave:
					// Get type of slave packet
					if (Slave_parse == undef)
					{
					LinClear(&lin_slave_transmit);
						if (Pull(&RS232_RX) == 0x01U)
						{
							Slave_parse = PID_compare;
						}
						else
						{
							Slave_parse = right_now;
						}
					}
					// Get parse input packet from VCP
					else
					{
						if (GetLinPacket(Pull(&RS232_RX), &lin_slave_transmit))
						{
							// Packet, when send after receive packet from PC
							if (Slave_parse == right_now)
							{
								LinDataFrameSend(&lin_slave_transmit);
								Slave_parse = undef;
								if (MUTE_MODE == 0xFFFFFFFFU)
								{
									lin_repeat_slave(&lin_slave_transmit);
								}
								parsedCommand = none_command;
							}
							// Packet, when will be send after compare PID value in the bus

							/*BUG HERE!!!*/
							else if(Slave_parse == PID_compare)
							{
								lin_slave_transmit.state = completed;
								Slave_parse = undef;
								for(uint8_t i = 0; i < 0xFF; ++i){__NOP();}
								parsedCommand = none_command;
							}

							else{
								//It's mistake case
								Clear(&RS232_RX);
								parsedCommand = none_command;
							}
							/*END PART OF BUG'S CODE*/
						}
					}
					break;
					/*******************************************************************************/
				case sendMaster:
					if (GetLinPacket(Pull(&RS232_RX), &lin_transmit))
					{
						if (MUTE_MODE == 0xFFFFFFFF)
						{
							lin_repeat_master(&lin_transmit);
						}
						LinSend(&lin_transmit);
						LinClear(&lin_transmit);
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
// This part of code i use for data transmit at IRQ TXE, IRQ disable in IRQ handler
#ifndef USB_VCP
		if (GetSize(&RS232_TX) != 0)
		{
			// Enable IRQ
			usart_interrupt_enable(USART0, USART_INT_TBE);
		}
#else
		// Send data from FIFO to usbd data buffer
		if (0 != GetSize(&RS232_TX))
		{
			uint8_t size = GetSize(&RS232_TX);
			for (uint8_t i = 0; i < size; ++i)
			{
				usb_data_buffer[i] = Pull(&RS232_TX);
			}
			cdc_acm_data_send(&usb_device_dev, size);
		}
#endif
		/*******************************************************************************/
		/*At this part of code we repat packets from LIN bus to the USBD VCP (LIN -> FIFO -> usb_data_buff)*/
		/*******************************************************************************/
		if (lin_received.state == completed)
		{ // Packet from LIN bus recognized, need to send to VCP
			if (lin_received.crc == lin_received.rcrc)
			{ // CRC is correctly, only repeat packet
				Push(&RS232_TX, lin_received.PID);
				for (uint8_t i = 0; i < lin_received.size; ++i)
				{
					Push(&RS232_TX, lin_received.data[i]);
				}
				Push(&RS232_TX, lin_received.crc);
				lin_received.state = wait_break;
			}
			else
			{ // Invalid CRC, insert 0x00 before packet
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
static inline void GetBackup(enum CRC_Type *crc, enum Filtering *filt, uint32_t *baud, uint32_t *mute, volatile uint32_t *pInfo, bool direction)
{
	if (direction)
	{ // Write info
		fmc_unlock();
		fmc_page_erase((uint32_t)pInfo);
		fmc_word_program((uint32_t)pInfo, *crc);
		fmc_word_program((uint32_t)pInfo + sizeof(uint32_t), *filt);
		fmc_word_program((uint32_t)pInfo + 2 * sizeof(uint32_t), *baud);
		fmc_word_program((uint32_t)pInfo + 3 * sizeof(uint32_t), *mute);
		fmc_lock();
	}
	else
	{
		*crc = (enum CRC_Type)pInfo[0];
		*filt = (enum Filtering)pInfo[1];
		*baud = (uint32_t)pInfo[2];
		*mute = (uint32_t)pInfo[3];
	}
}
// Check usbd data buffer
static inline void usbd_polling(void)
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
}

static inline void nvic_enable(void)
{
#ifndef USB_VCP
	print("LIN adapter ver. 1.0 2022-02-21\n\r");
	nvic_irq_enable(USART0_IRQn, 2, 1); // For UART0_PC
#else
	nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
	nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 1, 1);
#endif
	nvic_irq_enable(USART1_IRQn, 2, 2);	   // For LIN UART IRQ
	nvic_irq_enable(TIMER0_UP_IRQn, 3, 3); // For timming definition
}
