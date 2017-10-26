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

#define SOFT_RX_PIN		17
#define SOFT_TX_PIN  	18


#define BR 9600
#define TIMER_FREQ 								16000000
#define BAUDRATE_TO_TICKS(BR)   				\
					TIMER_FREQ/(BR*2)
					
#define TIMER_OFF 0
#define TIMER_ON_BY_RX 1
#define TIMER_ON_BY_TX 2
#define TIMER_ON_TXRX 3



void timer_init();
void rx_read();
void soft_uart_pins_init();
void tx_put();


#endif

