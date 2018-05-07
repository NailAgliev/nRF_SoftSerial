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
	void (*p_func)();
} sserial_t;


#define UART0_INSTANCE_INDEX 0






void rx_read(sserial_t * p_instance);
uint32_t tx_put(sserial_t	* p_instance);

void SoftSerial_init(sserial_t * p_instance, uint8_t tx_pin, uint8_t rx_pin, uint16_t baud_rate, uint8_t rx_bufer_size, uint8_t tx_bufer_size, void (* testfunc)(void));
uint32_t SSerial_put(sserial_t * p_instance, uint8_t * p_tx_byte);
uint32_t SSerial_put_from(sserial_t * p_instance, sserial_t * p_secin);
void SSerial_put_string(sserial_t * p_instance,char * p_string);

uint32_t SSerial_get(sserial_t * p_instance, uint8_t *p_byte);
void SSerial_get_string(sserial_t * p_instance, uint8_t * p_byte);

//void SSerial_tx_send_string();

#endif

