
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "softserial.h"
#include "app_fifo.h"

#define UART_TIMERS 5



extern sserial_t my_uart = 
{												
	.__tx_pin = 0,		
	.__rx_pin = 0,		
	.timer_tics = 0,
	.rx_byte = 0,
	.tx_byte = 0,
	.rx_half_bit_counter = 0,
	.tx_half_bit_counter = 0,
	.rx_counter = 0,
	.tx_counter = 0,
	.rx_fifo = 0,
	.tx_fifo = 0,
	.(*p_func)(void);
};

extern sserial_t uart = 
{												
.__tx_pin = 0,		
.__rx_pin = 0,		
.timer_tics = 0
};

uint8_t byte = 0;

//static sserial_t uart;

void testfunc()
		{
			SSerial_get(&byte);
			//SEGGER_RTT_printf(0, "%c", byte);
			SSerial_put(&byte);
		}


/**
 * @brief Function for main application entry.
 */
int main(void)
{
    
    //Configure all leds on board.
    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
		SoftSerial_init(&my_uart, 12, 18, 9600, 64, 64, &testfunc);
		SoftSerial_init(&uart, 12, 18, 9600, 64, 64, &testfunc);
		
		while(1)
    {
			__WFI();
		}
}

/** @} */
