#include "softserial.h"

uint8_t index = 0;
uint8_t rx_char = 0;
uint8_t tx_char = 0;
uint8_t rx_data[20];
uint8_t rx_half_bit_counter = 0;
uint8_t tx_half_bit_counter = 0;
uint8_t rx_counter = 0;
uint8_t tx_counter = 0;
uint8_t timer_flag = 0;
char tx_byte_for_send = 's';

const nrf_drv_timer_t UART_TIMER = NRF_DRV_TIMER_INSTANCE(0);
uint32_t err_code = NRF_SUCCESS;

void tx_put(void)
{
	tx_char = tx_byte_for_send;
	if(timer_flag == TIMER_OFF)
	{
		nrf_gpio_pin_clear(SOFT_TX_PIN);
		timer_flag = TIMER_ON_BY_TX;
		nrf_drv_timer_enable(&UART_TIMER);
		
		
	}
	else
	{
		timer_flag = TIMER_ON_TXRX;
	}
}

void tx_pin_set()
{
	if((tx_half_bit_counter >= 2) && ((tx_half_bit_counter % 2) == 0))
	{
		if((tx_char & ( 1 << tx_counter)) > 0)
		{	
			nrf_gpio_pin_set(SOFT_TX_PIN);
		}
		else 
		{
			nrf_gpio_pin_clear(SOFT_TX_PIN);
		}
		
		tx_counter++;
	}
	if(tx_half_bit_counter >17 )
	{
		tx_half_bit_counter = 0;
		nrf_drv_timer_disable(&UART_TIMER);
		nrf_gpio_pin_set(SOFT_TX_PIN);
		
		timer_flag = 0;
	}	
	
	
	
}

void rx_read()
{
	if(rx_half_bit_counter > 17)   // получили 8 бит данных
			{
					rx_half_bit_counter = 0;
					nrf_drv_gpiote_in_event_enable(SOFT_RX_PIN, true);// начинаем ждать стартовый бит
					nrf_drv_timer_disable(&UART_TIMER); // выключаем таймер
					rx_data[index] = rx_char;
					index++;
					rx_char = 0;
					rx_counter = 0;
					timer_flag = 0;
				  if(rx_char == 0xD) //конец посылки
				  {
						index = 0;
				  }
			}
	if((rx_half_bit_counter >= 3) && ((rx_half_bit_counter % 2) == 1)) // заходим по нечетным прерываниям (8 бит данных)
	{
	   if(nrf_drv_gpiote_in_is_set(SOFT_RX_PIN))
		 {
			 rx_char = rx_char | (1 << rx_counter); 
		 }
		 SEGGER_RTT_printf(0, "%x   %d \n\r", rx_char, rx_counter);
			rx_counter++;
	}
}
void rx_start_bit_handler (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(timer_flag == TIMER_OFF)
		{
		nrf_drv_timer_enable(&UART_TIMER);
		nrf_drv_gpiote_in_event_disable(SOFT_RX_PIN);
		SEGGER_RTT_printf(0, "start bit interupt\n\r");
		timer_flag = TIMER_ON_BY_RX;
		}
		else //если таймер запущенн то не включать таймер
	{
		nrf_drv_gpiote_in_event_disable(SOFT_RX_PIN);
		SEGGER_RTT_printf(0, "flag=1 \n\r");
		timer_flag = TIMER_ON_TXRX;
	}
}
void soft_uart_pins_init()
{
	nrf_drv_gpiote_init();
	nrf_drv_gpiote_in_config_t rx_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
	rx_config.pull = NRF_GPIO_PIN_PULLUP;
	
	nrf_drv_gpiote_in_init (SOFT_RX_PIN, &rx_config, rx_start_bit_handler);
	nrf_drv_gpiote_in_event_enable(SOFT_RX_PIN, true);
	
	nrf_gpio_cfg_output(SOFT_TX_PIN);
	nrf_gpio_pin_set(SOFT_TX_PIN);

}
void timer_uart_event_handler(nrf_timer_event_t event_type, void* p_context)
	{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
					switch (timer_flag)
					{
						case TIMER_ON_BY_RX:
						rx_half_bit_counter++;
						rx_read();
					break;
						case TIMER_ON_BY_TX:
						tx_half_bit_counter++;
						tx_pin_set();
					break;
						case TIMER_ON_TXRX:
						rx_half_bit_counter++;
						rx_read();
						tx_half_bit_counter++;
						tx_pin_set();
          break;
					}

        default:
            //Do nothing.
            break;
    }
	}
void timer_init(void)
{
		SEGGER_RTT_printf(0, "Timer init");
	
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
		{
					SEGGER_RTT_printf(0, " SUCCESS \n\r");
		}
			
		nrf_drv_timer_extended_compare(&UART_TIMER, NRF_TIMER_CC_CHANNEL0, BAUDRATE_TO_TICKS(BR), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);    

}
