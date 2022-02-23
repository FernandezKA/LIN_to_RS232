#pragma once

#include <stdint.h>
//#include <stdbool.h>

// GD Includes
#include <gd32f10x.h>
#include <gd32f10x_rcu.h>
#include <gd32f10x_usart.h>
#include <gd32f10x_timer.h>
#include <gd32f10x_misc.h>
#include <gd32f10x_usart.h>
#include <gd32f10x_fmc.h>

// User includes
#include "fifo.h"


#define USART_LIN USART1
#define USART_RS232 USART0
// User global variables
extern struct fifo LIN_RX;
extern struct fifo LIN_TX;
extern struct fifo RS232_RX;
extern struct fifo RS232_TX;
extern uint32_t SysCounter;
extern uint16_t BAUDRATE_LIN;

extern volatile uint32_t*  infoPage;

enum CRC_Type{
Enhanced, 
Classic
};

enum Filtering{
Hide_invalid, 
Show_invalid
};

enum Slave_type{
right_now, 
PID_compare, 
undef
};


extern enum CRC_Type CRC_parse;
extern enum Filtering Filtering_parse;
extern enum Slave_type Slave_parse;

// User definitions

// User function definition
void CLK_Init(void);
void GPIO_Init(void);
void USART0_Init(void);
void USART1_Init(void);
void TIM0_Init(void);
