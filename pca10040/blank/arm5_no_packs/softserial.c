#include "softserial.h"
#include "app_fifo.h"







uint8_t timer_flag = 0;
int32_t remaining_bytes_to_send = 0;    // количество байт которые предстоит отправить (сейчас не используется)

typedef struct
{
	uint8_t	rx_half_bit_counter;
	uint8_t	tx_half_bit_counter;
	uint8_t	timer_flag;
	
}uart_control_block;


const nrf_drv_timer_t UART_TIMER = NRF_DRV_TIMER_INSTANCE(0);
const nrf_drv_timer_t UART_TIMER1 = NRF_DRV_TIMER_INSTANCE(1);
const nrf_drv_timer_t UART_TIMER2 = NRF_DRV_TIMER_INSTANCE(2);
const nrf_drv_timer_t UART_TIMER3 = NRF_DRV_TIMER_INSTANCE(3);

uint8_t timer_init_flag = 0;

uint32_t err_code = NRF_SUCCESS;

void SoftSerial_init(sserial_t *  p_instance,	uint8_t tx_pin, uint8_t rx_pin, uint16_t baud_rate, uint8_t rx_bufer_size, uint8_t tx_bufer_size, uint32_t (*testfunc)())
{			
		p_instance->p_func = testfunc;
		p_instance->__tx_pin = tx_pin;
		p_instance->__rx_pin = rx_pin;
		p_instance->timer_tics = (TIMER_FREQ/(baud_rate*2));
		p_instance->rx_byte = 0;
		p_instance->tx_byte = 0;
		p_instance->rx_half_bit_counter = 0;
		p_instance->tx_half_bit_counter = 0;
		p_instance->rx_counter = 0;
		p_instance->tx_counter = 0;
		if(timer_init_flag == 0)
			{
			timer_init(p_instance, NULL, NULL, NULL);
				soft_uart_pins_init(p_instance, NULL, NULL, NULL);
				timer_init_flag++;
		}
		else if(timer_init_flag == 1)
			{
			timer_init(NULL, p_instance, NULL, NULL);
			soft_uart_pins_init(NULL, p_instance, NULL, NULL);
				timer_init_flag++;
		}
		else if(timer_init_flag == 2)
			{
			timer_init(NULL,	p_instance, NULL, NULL);
			soft_uart_pins_init(NULL,	p_instance, NULL, NULL);
				timer_init_flag++;
		}
		else if(timer_init_flag == 3)			
			{
			timer_init(NULL, NULL, NULL, p_instance);
			soft_uart_pins_init(NULL, NULL, NULL, p_instance);
				timer_init_flag++;
		}
		uint8_t rx_buffer[rx_bufer_size];
		uint8_t tx_buffer[tx_bufer_size];
		app_fifo_init(&p_instance->rx_fifo, rx_buffer, rx_bufer_size);
		app_fifo_init(&p_instance->tx_fifo, tx_buffer, tx_bufer_size);		
}
uint32_t SSerial_get(sserial_t * p_instance, uint8_t * p_byte)
{
	err_code = app_fifo_get(&p_instance->rx_fifo, p_byte);
	//SEGGER_RTT_printf(0, "%x", *p_byte);
	return err_code;
}

void SSerial_put_string(sserial_t * p_instance, uint8_t * p_string)
{
	//SEGGER_RTT_printf(0, "%s", p_string);
	for (uint8_t c=0; c < strlen(p_string); c++)
	{
		SSerial_put(p_instance, p_string+c);
	}
}



uint32_t SSerial_put(sserial_t * p_instance, uint8_t * p_tx_byte)
{
	err_code = app_fifo_put(&p_instance->tx_fifo, *p_tx_byte); //пихаем байт в фифо
	if(err_code == NRF_SUCCESS)
	{
 	if(p_instance->tx_byte == 0)
		{
			err_code = app_fifo_get(&p_instance->tx_fifo, &p_instance->tx_byte); // пихаем из фифо в байт tx
		if(err_code == NRF_ERROR_NOT_FOUND)
			{
				app_fifo_flush(&p_instance->tx_fifo);
				return NRF_SUCCESS;
			}
		}
	}
	else
	{
		return NRF_ERROR_NO_MEM;
	}
	tx_put(p_instance);
	return err_code;
}
uint32_t SSerial_put_from(sserial_t * p_instance, sserial_t * p_second_instance)
{
	uint8_t byte_between = 0;
	err_code = app_fifo_get(&p_instance->tx_fifo, &byte_between);
	if(err_code == NRF_ERROR_NOT_FOUND)
	{
		return err_code;
	}
	err_code = app_fifo_put(&p_second_instance->rx_fifo, byte_between);
	
	return err_code;
}



uint32_t tx_put(sserial_t	* p_instance)
{
	if(timer_flag == TIMER_OFF)
	{
		nrf_gpio_pin_clear(p_instance->__tx_pin);
		timer_flag = TIMER_ON_BY_TX;
		nrf_drv_timer_enable(&UART_TIMER);
		err_code = NRF_SUCCESS;
	}
	else if(timer_flag != TIMER_ON_BY_TX)
	{
		if(p_instance->tx_half_bit_counter == 0)
			{
				nrf_gpio_pin_clear(p_instance->__tx_pin);
			}
		
		timer_flag = TIMER_ON_TXRX;
		err_code = NRF_ERROR_BUSY;
	}
	return err_code;
}

void tx_pin_set(sserial_t	* p_instance)
{
	if(p_instance->tx_half_bit_counter > 17 ) // отправляем стоповый бит
	{
		nrf_gpio_pin_set(p_instance->__tx_pin);
	}		
	if(p_instance->tx_half_bit_counter > 19 ) 
	{
		p_instance->tx_half_bit_counter = 0;
		p_instance->tx_byte = 0;
		p_instance->tx_counter = 0;
		err_code = app_fifo_read(&p_instance->tx_fifo, NULL, &remaining_bytes_to_send); // проверяем нужно ли отправить ещё один байт и если не нужно выключаем таймер
		if(err_code == NRF_ERROR_NOT_FOUND)
			{
				if(timer_flag != TIMER_ON_TXRX)
					{
						nrf_drv_timer_disable(&UART_TIMER);
						timer_flag = TIMER_OFF;
					}
				else
					{
						timer_flag = TIMER_ON_BY_RX; 
					}
			}
		else 																		//если нужно отправить ещё один байт то отправляем стартовый бит
		{
			remaining_bytes_to_send--;
			app_fifo_get(&p_instance->tx_fifo, &p_instance->tx_byte);
			nrf_gpio_pin_clear(p_instance->__tx_pin);
		}
		
	}
	
	
	
	if((p_instance->tx_half_bit_counter >= 2) && ((p_instance->tx_half_bit_counter % 2) == 0) && (p_instance->tx_half_bit_counter <17)) // отправляем байты с 0 по 8
	{
		if((p_instance->tx_byte & ( 1 << p_instance->tx_counter)) > 0)
		{	
			nrf_gpio_pin_set(p_instance->__tx_pin);
			SEGGER_RTT_printf(0, "1");
		}
		else 
		{
			SEGGER_RTT_printf(0, "0");
			nrf_gpio_pin_clear(p_instance->__tx_pin);
		}
		p_instance->tx_counter++;
	}
}

void rx_read(sserial_t * p_instance)
{
	
	if(p_instance->rx_half_bit_counter > 17)   // получили 8 бит данных
			{
					p_instance->rx_half_bit_counter = 0;
					nrf_drv_gpiote_in_event_enable(p_instance->__rx_pin, true);// начинаем ждать стартовый бит
					if(timer_flag != TIMER_ON_TXRX)
						{
							nrf_drv_timer_disable(&UART_TIMER);
							nrf_drv_timer_disable(&UART_TIMER1);
							nrf_drv_timer_disable(&UART_TIMER2);
							nrf_drv_timer_disable(&UART_TIMER3);
							timer_flag = TIMER_OFF;
						}
					else
						{
							timer_flag = TIMER_ON_BY_TX;
						}
					p_instance->rx_counter = 0;
					SEGGER_RTT_printf(0, "%c", p_instance->rx_byte);
					app_fifo_put(&p_instance->rx_fifo, p_instance->rx_byte);
					p_instance->p_func();
					p_instance->rx_byte = 0;				
			}
	if((p_instance->rx_half_bit_counter >= 3) && ((p_instance->rx_half_bit_counter % 2) == 1)) // заходим по нечетным прерываниям (8 бит данных)
	{
	   if(nrf_drv_gpiote_in_is_set(p_instance->__rx_pin))
		 {
			 p_instance->rx_byte = p_instance->rx_byte | (1 << p_instance->rx_counter); 
		 }
			p_instance->rx_counter++;
	}
}
void rx_start_bit_handler (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(timer_flag == TIMER_OFF)
		{
		nrf_drv_timer_enable(&UART_TIMER);
		timer_flag = TIMER_ON_BY_RX;
		}
		else if(timer_flag != TIMER_ON_BY_RX) //если таймер запущенн то не включать таймер
	{
		timer_flag = TIMER_ON_TXRX;
	}
}
void rx_start_bit_handler1 (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(timer_flag == TIMER_OFF)
		{
		nrf_drv_timer_enable(&UART_TIMER1);
		timer_flag = TIMER_ON_BY_RX;
		}
		else if(timer_flag != TIMER_ON_BY_RX) //если таймер запущенн то не включать таймер
	{
		timer_flag = TIMER_ON_TXRX;
	}
}
void rx_start_bit_handler2 (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(timer_flag == TIMER_OFF)
		{
		nrf_drv_timer_enable(&UART_TIMER2);
		timer_flag = TIMER_ON_BY_RX;
		}
		else if(timer_flag != TIMER_ON_BY_RX) //если таймер запущенн то не включать таймер
	{
		timer_flag = TIMER_ON_TXRX;
	}
}
void rx_start_bit_handler3 (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
	if(timer_flag == TIMER_OFF)
		{
		nrf_drv_timer_enable(&UART_TIMER3);
		timer_flag = TIMER_ON_BY_RX;
		}
		else if(timer_flag != TIMER_ON_BY_RX) //если таймер запущенн то не включать таймер
	{
		timer_flag = TIMER_ON_TXRX;
	}
}
void soft_uart_pins_init(sserial_t * p_instance, sserial_t * p_instance1, sserial_t * p_instance2, sserial_t * p_instance3)
{
	nrf_drv_gpiote_init();
	nrf_drv_gpiote_in_config_t rx_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
	rx_config.pull = NRF_GPIO_PIN_PULLUP;
	if(p_instance != NULL)
	{
	nrf_drv_gpiote_in_init (p_instance->__rx_pin, &rx_config, rx_start_bit_handler);
	nrf_drv_gpiote_in_event_enable(p_instance->__rx_pin, true);
	nrf_gpio_cfg_output(p_instance->__tx_pin);
	nrf_gpio_pin_set(p_instance->__tx_pin);
	}
	if(p_instance1 != NULL)
	{
	nrf_drv_gpiote_in_init (p_instance1->__rx_pin, &rx_config, rx_start_bit_handler1);
	nrf_drv_gpiote_in_event_enable(p_instance1->__rx_pin, true);
	nrf_gpio_cfg_output(p_instance1->__tx_pin);
	nrf_gpio_pin_set(p_instance1->__tx_pin);
	}
	if(p_instance2 != NULL)
	{
	nrf_drv_gpiote_in_init (p_instance2->__rx_pin, &rx_config, rx_start_bit_handler2);
	nrf_drv_gpiote_in_event_enable(p_instance2->__rx_pin, true);
	nrf_gpio_cfg_output(p_instance2->__tx_pin);
	nrf_gpio_pin_set(p_instance2->__tx_pin);
	}
	if(p_instance3 != NULL)
	{
	nrf_drv_gpiote_in_init (p_instance3->__rx_pin, &rx_config, rx_start_bit_handler3);
	nrf_drv_gpiote_in_event_enable(p_instance3->__rx_pin, true);
	nrf_gpio_cfg_output(p_instance3->__tx_pin);
	nrf_gpio_pin_set(p_instance3->__tx_pin);
	}

}
void timer_uart_event_handler(nrf_timer_event_t event_type, sserial_t * p_context)
	{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
					switch (timer_flag)
					{
						case TIMER_ON_BY_RX:
								if(p_context->rx_half_bit_counter < 1)
							{
								nrf_drv_gpiote_in_event_disable(p_context->__rx_pin);
							}
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
					break;
						case TIMER_ON_BY_TX:
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
					break;
						case TIMER_ON_TXRX:
						if(p_context->rx_half_bit_counter < 1)
							{
								nrf_drv_gpiote_in_event_disable(p_context->__rx_pin);
							}
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
          break;
					}

        default:
            //Do nothing.
            break;
    }
	}
void timer_uart_event_handler1(nrf_timer_event_t event_type, sserial_t * p_context)
	{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
					switch (timer_flag)
					{
						case TIMER_ON_BY_RX:
						if(p_context->rx_half_bit_counter < 1)
							{
								nrf_drv_gpiote_in_event_disable(p_context->__rx_pin);
							}
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
					break;
						case TIMER_ON_BY_TX:
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
					break;
						case TIMER_ON_TXRX:
						if(p_context->rx_half_bit_counter < 1)
							{
								nrf_drv_gpiote_in_event_disable(p_context->__rx_pin);
							}
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
          break;
					}

        default:
            //Do nothing.
            break;
    }
	}
void timer_uart_event_handler2(nrf_timer_event_t event_type, sserial_t * p_context)
	{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
					switch (timer_flag)
					{
						case TIMER_ON_BY_RX:
						if(p_context->rx_half_bit_counter < 1)
							{
								nrf_drv_gpiote_in_event_disable(p_context->__rx_pin);
							}
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
					break;
						case TIMER_ON_BY_TX:
						if(p_context->rx_half_bit_counter < 1)
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
					break;
						case TIMER_ON_TXRX:
						if(p_context->rx_half_bit_counter < 1)
							{
								nrf_drv_gpiote_in_event_disable(p_context->__rx_pin);
							}
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
          break;
					}

        default:
            //Do nothing.
            break;
    }
	}
void timer_uart_event_handler3(nrf_timer_event_t event_type, sserial_t * p_context)
	{
    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
					switch (timer_flag)
					{
						case TIMER_ON_BY_RX:
						if(p_context->rx_half_bit_counter < 1)
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
					break;
						case TIMER_ON_BY_TX:
						if(p_context->rx_half_bit_counter < 1)
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
					break;
						case TIMER_ON_TXRX:
						if(p_context->rx_half_bit_counter < 1)
							{
								nrf_drv_gpiote_in_event_disable(p_context->__rx_pin);
							}
						p_context->rx_half_bit_counter++;
						rx_read(p_context);
						p_context->tx_half_bit_counter++;
						tx_pin_set(p_context);
          break;
					}

        default:
            //Do nothing.
            break;
    }
	}
void timer_init(sserial_t * p_instance, sserial_t * p_instance1, sserial_t * p_instance2, sserial_t * p_instance3)
{	
		if(p_instance != NULL)
			{
		nrf_drv_timer_config_t timer_cfg = 
		{
	  .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz,
    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16 ,
    .interrupt_priority = TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    
    .p_context          = p_instance
		};
		
		err_code =	nrf_drv_timer_init(&UART_TIMER, &timer_cfg, timer_uart_event_handler);
		if(err_code == NRF_SUCCESS)
		SEGGER_RTT_printf(0, "TIMER0\n\r");
		nrf_drv_timer_extended_compare(&UART_TIMER, NRF_TIMER_CC_CHANNEL0, p_instance->timer_tics, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
		}
		else if(p_instance1 != NULL)
			{
		nrf_drv_timer_config_t timer_cfg1 =
		{
	  .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz,
    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16 ,
    .interrupt_priority = TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    
    .p_context          = p_instance1
		};
		
		err_code = nrf_drv_timer_init(&UART_TIMER1, &timer_cfg1, timer_uart_event_handler1);
		if(err_code == NRF_SUCCESS)
		SEGGER_RTT_printf(0, "TIMER1\n\r");
		nrf_drv_timer_extended_compare(&UART_TIMER1, NRF_TIMER_CC_CHANNEL0, p_instance1->timer_tics, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
		}
		else	if(p_instance2 != NULL)
			{
		nrf_drv_timer_config_t timer_cfg2 =
		{
	  .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz,
    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16 ,
    .interrupt_priority = TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    
    .p_context          = p_instance2
		};
		err_code = nrf_drv_timer_init(&UART_TIMER2, &timer_cfg2, timer_uart_event_handler2);
		SEGGER_RTT_printf(0, "%s", err_code);
		nrf_drv_timer_extended_compare(&UART_TIMER2, NRF_TIMER_CC_CHANNEL0, p_instance2->timer_tics, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
			}
		else	if(p_instance3 != NULL)
			{	
		nrf_drv_timer_config_t timer_cfg3 =
		{
	  .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz,
    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16 ,
    .interrupt_priority = TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    
    .p_context          = p_instance3
		};
		err_code = nrf_drv_timer_init(&UART_TIMER3, &timer_cfg3, timer_uart_event_handler3);
		SEGGER_RTT_printf(0, "%s", err_code);
		nrf_drv_timer_extended_compare(&UART_TIMER3, NRF_TIMER_CC_CHANNEL0, p_instance3->timer_tics, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);	
			}
			

}



