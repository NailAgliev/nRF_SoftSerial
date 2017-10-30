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
#define BAUDRATE_TO_TICKS(BR)   				\
					TIMER_FREQ/(BR*2)
					
#define TIMER_OFF 0
#define TIMER_ON_BY_RX 1
#define TIMER_ON_BY_TX 2
#define TIMER_ON_TXRX 3


void timer_init();
void rx_read();
void soft_uart_pins_init();
uint32_t tx_put();
void SoftSerial_init(uint8_t tx_pin, uint8_t rx_pin, uint16_t baud_rate, uint8_t rx_bufer_size, uint8_t tx_bufer_size);
uint32_t SSerial_put(uint8_t * p_tx_byte);
void SSerial_put_string(uint8_t * p_string);

void SSerial_get(uint8_t *p_byte);
//void SSerial_tx_send_string();

#endif

