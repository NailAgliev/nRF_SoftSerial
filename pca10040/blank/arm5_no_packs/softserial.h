#ifndef SOFTSETIAL_H__
#define SOFTSERIAL_H__


#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "bsp.h"
#include "app_error.h"
#include "nrf_drv_gpiote.h"
#include "SEGGER_RTT.h"
#include "app_fifo.h"
#include "string.h"


#define TIMER_FREQ 						16000000

#define TIMER_OFF 0
#define TIMER_ON_BY_RX 1
#define TIMER_ON_BY_TX 2
#define TIMER_ON_TXRX 3




#define TEST_MACRO(name1, name2) name1##name2
#define UART_INIT(index) CONCAT_2(tx_byte, index);

typedef struct
{
	uint8_t __tx_pin;
	uint8_t __rx_pin;
	uint16_t timer_tics;
	uint8_t rx_byte;
	uint8_t tx_byte;
	uint8_t rx_half_bit_counter;
	uint8_t tx_half_bit_counter;
	uint8_t rx_counter;
	uint8_t tx_counter;
	app_fifo_t rx_fifo;
	app_fifo_t tx_fifo;
	uint32_t (*p_func)();
} sserial_t;


#define SSERIAL_DEFALUT				\
{															\
	.__tx_pin = 0,							\
	.__rx_pin = 0,							\
	.timer_tics = 0,						\
	.rx_byte = 0,    						\
	.tx_byte = 0,								\
	.rx_half_bit_counter = 0, 	\
	.tx_half_bit_counter = 0, 	\
	.rx_counter = 0,   					\
	.tx_counter = 0,  					\
	.rx_fifo = 0,  							\
	.tx_fifo = 0,  							\
}

#define UART0_INSTANCE_INDEX 0
#define UART1_INSTANCE_INDEX UART0_INSTANCE_INDEX
#define UART2_INSTANCE_INDEX UART1_INSTANCE_INDEX
#define UART3_INSTANCE_INDEX UART2_INSTANCE_INDEX
#define UART4_INSTANCE_INDEX UART3_INSTANCE_INDEX
#define UART5_INSTANCE_INDEX UART4_INSTANCE_INDEX
#define UART6_INSTANCE_INDEX UART5_INSTANCE_INDEX





void timer_init(sserial_t * p_instance);
void rx_read(sserial_t * p_instance);
void soft_uart_pins_init(sserial_t * p_instance);
uint32_t tx_put(sserial_t	* p_instance);
void SoftSerial_init(sserial_t * p_instance, uint8_t tx_pin, uint8_t rx_pin, uint16_t baud_rate, uint8_t rx_bufer_size, uint8_t tx_bufer_size, uint32_t (* testfunc)(void));
uint32_t SSerial_put(sserial_t * p_instance, uint8_t * p_tx_byte);
uint32_t SSerial_put_from(sserial_t * p_instance, sserial_t * p_secin);
void SSerial_put_string(sserial_t * p_instance,uint8_t * p_string);

uint32_t SSerial_get(sserial_t * p_instance, uint8_t *p_byte);
//void SSerial_tx_send_string();

#endif

