
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
	.tx_fifo = 0
};


uint8_t byte = 0;

void testfunc()
		{
			SSerial_get(&my_uart, &byte);
			SSerial_put(&my_uart, &byte);
		}
int main(void)
{
		SoftSerial_init(&my_uart, 12, 18, 9600, 64, 64, &testfunc);
		
		while(1)
    {
			__WFI();
		}
}

/** @} */
