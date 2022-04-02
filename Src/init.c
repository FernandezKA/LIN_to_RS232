#include "main.h"
#include <gd32f10x_usart.h>
//Init CLK at 72 MHz of clocked, with PLL
void CLK_Init(void) {}
//Only set gpio - work modes
void GPIO_Init(void)
{
	RCU_APB2EN |= RCU_APB2EN_PCEN | RCU_APB2EN_PAEN;
	RCU_APB2EN |= RCU_APB2EN_USART0EN;

	rcu_periph_clock_enable(RCU_USBD);
	rcu_ckout0_config(RCU_CKOUT0SRC_CKSYS);
	rcu_usb_clock_config(RCU_CKUSB_CKPLL_DIV1_5);
	gpio_init(GPIOC, GPIO_MODE_OUT_OD, GPIO_OSPEED_10MHZ, GPIO_PIN_13); // It's led for indicate activity
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);	// usb
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);	// usb
}
// This uart used for RS232 communicate
void USART0_Init(void)
{

	usart_deinit(USART0);
	usart_baudrate_set(USART0, 115200UL);
	usart_parity_config(USART0, USART_PM_NONE);
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
	usart_receive_config(USART0, USART_RECEIVE_ENABLE);
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	gpio_afio_deinit();
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	usart_enable(USART0);
}
// This uart used for LIN communication
void USART1_Init(void)
{
	RCU_APB1EN |= RCU_APB1EN_USART1EN;
	RCU_APB2EN |= RCU_APB2EN_PAEN;
	usart_deinit(USART1);
	usart_baudrate_set(USART1, 9600UL);
	usart_parity_config(USART1, USART_PM_NONE);
	usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
	usart_receive_config(USART1, USART_RECEIVE_ENABLE);
	usart_interrupt_enable(USART1, USART_INT_RBNE);
	usart_interrupt_enable(USART1, USART_INT_LBD); // Lin break detect
	usart_lin_mode_enable(USART1);
	USART_CTL1(USART1)|=USART_CTL1_LBLEN;
	gpio_afio_deinit();
	gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
	gpio_init(GPIOA, GPIO_MODE_IPD, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
	usart_enable(USART1);
}
// This timer used for led indicating
void TIM0_Init(void)
{
	RCU_APB2EN |= RCU_APB2EN_TIMER0EN;
	TIMER_CTL0(TIMER0) |= TIMER_CTL0_CEN;
	TIMER_PSC(TIMER0) = 71999;
	TIMER_CAR(TIMER0) = 9999;
	TIMER_DMAINTEN(TIMER0) |= TIMER_DMAINTEN_UPIE;
}
