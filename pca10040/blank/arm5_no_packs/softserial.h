#ifndef SOFTSETIAL_H__
#define SOFTSERIAL_H__


#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "nrf_drv_timer.h"
#include "bsp.h"
#include "app_error.h"

void timer_init();


#define BR 9600

#define TIMER_FREQ 								16000000
#define BAUDRATE_TO_TICKS(BR)   				\
					TIMER_FREQ/(BR*2)
					

#endif