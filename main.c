
#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "softserial.h"
#include "app_fifo.h"

#define UART_TIMERS 5



static sserial_t termite;
static sserial_t sim800;




uint8_t byte = 0;

uint8_t test_string[256];

uint8_t n = 0;

void testfunc()
		{
			SSerial_get(&termite, &byte);
			SSerial_put(&termite, &byte);
			//SSerial_get_string(&arduino, test_string);
//			n = sizeof(test_string);
//			SEGGER_RTT_printf(0, "%d", n);
		//	SSerial_put_string(&arduino, test_string);
		}
int main(void)
{
		SoftSerial_init(&termite,16, 18, 9600, 64, 64, &testfunc);
		while(1)
    {
			__WFI();
		}
}

/** @} */
