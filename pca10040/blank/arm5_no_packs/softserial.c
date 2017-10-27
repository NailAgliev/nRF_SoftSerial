#include "softserial.h"
#include "app_fifo.h"


uint8_t __tx_pin = 0;
uint8_t __rx_pin = 0;

uint16_t timer_tics = 0;

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
app_fifo_t rx_fifo;
app_fifo_t tx_fifo;

const nrf_drv_timer_t UART_TIMER = NRF_DRV_TIMER_INSTANCE(0);
uint32_t err_code = NRF_SUCCESS;

void SoftSerial_init(uint8_t tx_pin, uint8_t rx_pin, uint16_t baud_rate, uint8_t rx_bufer_size, uint8_t tx_bufer_size)
{
			
		__tx_pin = tx_pin;
		__rx_pin = rx_pin;
		timer_tics = (TIMER_FREQ/(baud_rate*2));
		timer_init();
		soft_uart_pins_init();
		uint8_t rx_buffer[rx_bufer_size];
		uint8_t tx_buffer[tx_bufer_size];
		app_fifo_init(&rx_fifo, rx_buffer, rx_bufer_size);
		app_fifo_init(&tx_fifo, tx_buffer, tx_bufer_size);		
}
void SSerial_get(uint8_t * p_byte)
{
	app_fifo_get(&rx_fifo, p_byte);
}

void SSerial_put(uint8_t * p_tx_byte)
{
	app_fifo_put(&tx_fifo, *p_tx_byte);
}
void tx_put(void)
{
	if(timer_flag == TIMER_OFF)
	{
		nrf_gpio_pin_clear(__tx_pin);
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
			nrf_gpio_pin_set(__tx_pin);
		}
		else 
		{
			nrf_gpio_pin_clear(__tx_pin);
		}
		tx_counter++;
	}
	if(tx_half_bit_counter >17 )
	{
		tx_half_bit_counter = 0;
		if(timer_flag != TIMER_ON_TXRX)
		{
		nrf_drv_timer_disable(&UART_TIMER);
		timer_flag = TIMER_OFF;
		}
		else
		{
			timer_flag = TIMER_ON_BY_RX;
		}
		
		nrf_gpio_pin_set(__tx_pin);
	}	
}

void rx_read()
{
	if(rx_half_bit_counter > 17)   // получили 8 бит данных
			{
					rx_half_bit_counter = 0;
					nrf_drv_gpiote_in_event_enable(__rx_pin, true);// начинаем ждать стартовый бит
					if(timer_flag != TIMER_ON_TXRX)
						{
							nrf_drv_timer_disable(&UART_TIMER);
							timer_flag = TIMER_OFF;
						}
					else
						{
							timer_flag = TIMER_ON_BY_TX;
						}
					//SEGGER_RTT_printf(0, "%x\n\r", rx_char);
					rx_data[index] = rx_char;
					index++;
						  if(rx_char == 0xA) //конец посылки
				  {
						index = 0;
						//SEGGER_RTT_printf(0, "get A\n\r");
						SEGGER_RTT_printf(0, "%s", rx_data);
				  }
					
					rx_char = 0;
					rx_counter = 0;
				
			}
	if((rx_half_bit_counter >= 3) && ((rx_half_bit_counter % 2) == 1)) // заходим по нечетным прерываниям (8 бит данных)
	{
	   if(nrf_drv_gpiote_in_is_set(__rx_pin))
		 {
			 rx_char = rx_char | (1 << rx_counter); 
		 }
			rx_counter++;
	}
}
void rx_start_bit_handler (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(timer_flag == TIMER_OFF)
		{
		nrf_drv_timer_enable(&UART_TIMER);
		nrf_drv_gpiote_in_event_disable(__rx_pin);
		timer_flag = TIMER_ON_BY_RX;
		}
		else //если таймер запущенн то не включать таймер
	{
		nrf_drv_gpiote_in_event_disable(__rx_pin);
		timer_flag = TIMER_ON_TXRX;
	}
}
void soft_uart_pins_init()
{
	nrf_drv_gpiote_init();
	nrf_drv_gpiote_in_config_t rx_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
	rx_config.pull = NRF_GPIO_PIN_PULLUP;
	
	nrf_drv_gpiote_in_init (__rx_pin, &rx_config, rx_start_bit_handler);
	nrf_drv_gpiote_in_event_enable(__rx_pin, true);
	
	nrf_gpio_cfg_output(__tx_pin);
	nrf_gpio_pin_set(__tx_pin);

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
		nrf_drv_timer_config_t timer_cfg =
		{
	  .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz,
    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16 ,
    .interrupt_priority = TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    
    .p_context          = NULL     
		};

		nrf_drv_timer_init(&UART_TIMER, &timer_cfg, timer_uart_event_handler);

		nrf_drv_timer_extended_compare(&UART_TIMER, NRF_TIMER_CC_CHANNEL0, timer_tics, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);    

}



