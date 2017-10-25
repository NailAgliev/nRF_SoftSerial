#include "softserial.h"

//const nrf_drv_timer_t UART_TIMER = NRF_DRV_TIMER_INSTANCE(0);
//uint32_t err_code = NRF_SUCCESS;
//nrf_drv_timer_config_t timer_cfg = 
//{
//	  .frequency          = (nrf_timer_frequency_t)NRF_TIMER_FREQ_16MHz,
//    .mode               = (nrf_timer_mode_t)NRF_TIMER_MODE_TIMER,          
//    .bit_width          = (nrf_timer_bit_width_t)NRF_TIMER_BIT_WIDTH_16 ,
//    .interrupt_priority = TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,                    
//    .p_context          = NULL     
//};
//err_code = nrf_drv_timer_init(&UART_TIMER, &timer_cfg, timer_led_event_handler);
//APP_ERROR_CHECK(err_code);

//nrf_drv_timer_extended_compare(
//         &UART_TIMER, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);