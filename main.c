
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "softserial.h"
#include "app_fifo.h"

#define UART_TIMERS 5



static sserial_t termite;
static sserial_t arduino;


uint8_t byte = 0;

uint8_t test_string[256];

void testfunc()
		{
			//SSerial_get(&arduino, &byte);
			//SSerial_put(&arduino, &byte);
			SSerial_get_string(&arduino, test_string);
			SEGGER_RTT_printf(0, "%s", test_string);
			SSerial_put_string(&arduino, test_string);
		}
int main(void)
{
		//SoftSerial_init(&termite, 17, 20, 9600, 64, 64, &testfunc);
		SoftSerial_init(&arduino, 17, 18, 9600, 64, 64, &testfunc);
		while(1)
    {
			__WFI();
		}
}

/** @} */
