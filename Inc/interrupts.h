#pragma once

#include "main.h"
#include <stdint.h>
// This IRQ handler for LIN bus
void USART0_IRQHandler(void);
// This IRQ handler for RS232 bus
void USART1_IRQHandler(void);
// This IRQ handler for indicate activity
void TIMER0_UP_IRQHandler(void);
// This IRQ handler for usbd vcp
void USBD_LP_CAN0_RX0_IRQHandler(void);
