#include "softserial.h"




uint8_t index = 0;
uint8_t rx_char = 0;
uint8_t rx_data[20];
uint8_t half_uart_bit_counter = 0;
uint8_t counter = 0;
uint8_t test_rx_line = 0;
uint8_t bit_push = 0;

const nrf_drv_timer_t UART_TIMER = NRF_DRV_TIMER_INSTANCE(0);
uint32_t err_code = NRF_SUCCESS;

//void inv (void)
//{
//	uint8_t t= 0;
//	while (rx_char != 0)
//	{
//		t= t*2 + rx_char % 2;
//		rx_char/=2;
//	}
//	rx_char=t;
//}

void rx_read()
{
	if(half_uart_bit_counter > 17)   // получили 8 бит данных
			{
				
					half_uart_bit_counter = 0;
					nrf_drv_gpiote_in_event_enable(SOFT_RX_PIN, true);// начинаем ждать стартовый бит
					nrf_drv_timer_disable(&UART_TIMER); // выключаем таймер
					//SEGGER_RTT_printf(0, "%x \n\r", rx_char);
					rx_data[index] = rx_char;
					index++;
					//SEGGER_RTT_printf(0, "%x \n\r", rx_char);
					rx_char = 0;
					counter = 0;
				bit_push = 0;
				  if(rx_char == 0xD) //конец посылки
				  {
						
						//SEGGER_RTT_printf(0, "%s \n\r", rx_data);
						index = 0;
						
				  }
						
			}
	if((half_uart_bit_counter >= 3) && ((half_uart_bit_counter % 2) == 1)) // заходим по нечетным прерываниям (8 бит данных)
	{
		 	//	SEGGER_RTT_printf(0, "%d\n\r", half_uart_bit_counter);
		
	   if(nrf_drv_gpiote_in_is_set(SOFT_RX_PIN))
		 {
			 rx_char = rx_char | (1 << counter);
//			 test_rx_line = 1;
//			 bit_push |= (1 << counter);
//			  			 
//		 }
//		 else 
//		 {
//				test_rx_line = 0;
//		 }
//		 
//		 
//		 SEGGER_RTT_printf(0, "counter = %x, rx_line = %x, bit_push = %x\n\r",  counter, test_rx_line, bit_push);
//		 
//		 
//		 
		 }
		 SEGGER_RTT_printf(0, "%x   %d \n\r", rx_char, counter);
			counter++;
		 
			
	}
		
}




void rx_start_bit_handler (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(half_uart_bit_counter == 0){
	
	nrf_drv_timer_enable(&UART_TIMER);
	nrf_drv_gpiote_in_event_enable(SOFT_RX_PIN, false);
				SEGGER_RTT_printf(0, "start bit interupt\n\r");
	}
}

void rx_init()
{
	nrf_drv_gpiote_init();
	nrf_drv_gpiote_in_config_t rx_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
	rx_config.pull = NRF_GPIO_PIN_PULLUP;
	
	nrf_drv_gpiote_in_init (SOFT_RX_PIN, &rx_config, rx_start_bit_handler);
	nrf_drv_gpiote_in_event_enable(SOFT_RX_PIN, true);
}

void timer_uart_event_handler(nrf_timer_event_t event_type, void* p_context)
	{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
					half_uart_bit_counter++;		
				
				rx_read();
            break;

        default:
            //Do nothing.
            break;
    }
	}


void timer_init(void)
{
	
		nrf_drv_timer_config_t timer_cfg =
		{
	  .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz,
    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16 ,
    .interrupt_priority = TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    
    .p_context          = NULL     
		};

		err_code = nrf_drv_timer_init(&UART_TIMER, &timer_cfg, timer_uart_event_handler);
		APP_ERROR_CHECK(err_code);
		if(err_code==NRF_SUCCESS);
			

		nrf_drv_timer_extended_compare(&UART_TIMER, NRF_TIMER_CC_CHANNEL0, BAUDRATE_TO_TICKS(BR), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);    

}


