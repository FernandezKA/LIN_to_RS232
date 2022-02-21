#pragma once

#include <stdint.h>

// GD Includes
#include <gd32f10x.h>
#include <gd32f10x_rcu.h>
#include <gd32f10x_usart.h>
#include <gd32f10x_timer.h>
#include <gd32f10x_misc.h>
#include <gd32f10x_usart.h>

// User includes
#include "fifo.h"

// User global variables
extern struct fifo LIN_RX;
extern struct fifo LIN_TX;
extern struct fifo RS232_RX;
extern struct fifo RS232_TX;

extern uint32_t SysCounter;

extern uint16_t BAUDRATE_LIN;

// User definitions

// User function definition
void CLK_Init(void);
void GPIO_Init(void);
void USART0_Init(void);
void USART1_Init(void);
void TIM0_Init(void);
