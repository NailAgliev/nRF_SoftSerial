
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "softserial.h"
#include "app_fifo.h"

#define UART_TIMERS 5



static sserial_t my_uart = SSERIAL_DEFALUT;



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
