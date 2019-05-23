#ifndef __MAIN_H
#define	__MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "sdk_common.h"
#include "nrf_drv_spis.h"
#include "nrf_drv_config.h"
#include "nrf_drv_wdt.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "app_error.h"

#include "simple_uart.h"
#include "my_uart.h"
#include "my_misc_fun.h"
#include "my_radio.h"
#include "my_timer.h"
//#include "watchdog.h"
#include "my_spi.h"
#include "se2431l.h"
#include "ringbuf2.h"
#include "ringbuf_nrf.h"
#include "watchdog.h"
#include "flash.h"


// 软件版本 -----------------------------------------------------------
#define 	VERSION_LEVEL_1		(2)		//一级版本号
#define     VERSION_LEVEL_2		(6)		//二级版本号

// 示波器
#define SBQ_DEBUG_PIN_SET()		nrf_gpio_pin_set(TX_PIN_NUMBER_1)
#define SBQ_DEBUG_PIN_CLEAR()	nrf_gpio_pin_clear(TX_PIN_NUMBER_1)
#define SBQ_DEBUG_PIN_TOG()		nrf_gpio_pin_toggle(TX_PIN_NUMBER_1)

uint8_t XOR_Cal(uint8_t * dat,uint16_t length);
void gpio_default_init(void);
void clocks_start(void);
uint8_t get_random_number(void);


#endif 




