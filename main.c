
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "softserial.h"
#include "app_fifo.h"

#define UART_TIMERS 5



static sserial_t first_weigher = SSERIAL_DEFALUT;
static sserial_t second_weigher = SSERIAL_DEFALUT;
static sserial_t termite = SSERIAL_DEFALUT;


uint8_t byte = 0;

void testfunc()
		{
			SSerial_get(&first_weigher, &byte);
			SSerial_put(&first_weigher, &byte);
		}
int main(void)
{
		SoftSerial_init(&first_weigher, 17, 20, 9600, 64, 64, &testfunc);
		//SoftSerial_init(&second_weigher, 16, 19, 9600, 64, 64, &testfunc);
		while(1)
    {
			__WFI();
		}
}

/** @} */
