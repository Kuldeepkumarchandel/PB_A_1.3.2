#ifndef HEADER_H_
#define HEADER_H_



#define BATT_ADC                    GPIO_NUM_1
#define SCLL                        GPIO_NUM_42
#define SDAA                        GPIO_NUM_41

#define CHRG_CTRL                   GPIO_NUM_4
#define CHARGING_ENABLE             gpio_set_level(CHRG_CTRL,0);
#define CHARGING_DISABLE            gpio_set_level(CHRG_CTRL,1);
#define SWITCH_A                    GPIO_NUM_5        
#define SWITCH_B                    GPIO_NUM_6
#define SWITCH_C                    GPIO_NUM_7
#define SWITCH_D                    GPIO_NUM_8

#define SWITCH_1                    GPIO_NUM_9
#define SWITCH_2                    GPIO_NUM_10
#define SWITCH_3                    GPIO_NUM_11
#define SWITCH_4                    GPIO_NUM_12
#define SWITCH_5                    GPIO_NUM_13
#define SWITCH_6                    GPIO_NUM_14
#define SWITCH_7                    GPIO_NUM_18

#define ACC_INT1                    GPIO_NUM_17

#define LCD_SDA                     GPIO_NUM_37         
#define LCD_SCL                     GPIO_NUM_40

#define AUDIO_IN                    GPIO_NUM_39
#define AUDIO_BCLK                  GPIO_NUM_46
#define AUDIO_LRCLK                 GPIO_NUM_45

#define DS                          GPIO_NUM_48
// #define DS2                         GPIO_NUM_34
#define PWR_EN_595                  GPIO_NUM_47


#define RTC_SDA                     GPIO_NUM_34
#define RTC_SCL                     SCLL
#define RTC_RESET                   GPIO_NUM_33

#define STCP                        GPIO_NUM_35
#define SHCP                        GPIO_NUM_36
#define STCP1                       GPIO_NUM_38

#define BACKLIGHT                   GPIO_NUM_26

#define PWR_OFF_ON_SWITCH           GPIO_NUM_21
#define MODE_SWITCH                 GPIO_NUM_2
#define ALARM_SWITCH                GPIO_NUM_3

// #define ESP_TX                      GPIO_NUM_43
// #define ESP_RX                      GPIO_NUM_44


#define REFILL_SWITCH                 GPIO_NUM_44
#define BOTTLE_SWITCH                 GPIO_NUM_43


#define AUDIO_PWR_EN                GPIO_NUM_45
#define BOOST_PWR_EN                GPIO_NUM_46

// BACKLIGHT
#define BACKLIGHT_ON                gpio_set_level(BACKLIGHT,1);
#define BACKLIGHT_OFF               gpio_set_level(BACKLIGHT,0);

// 74HC595
#define PWR_EN_595_ON               gpio_set_level(PWR_EN_595, 0);
#define PWR_EN_595_OFF              gpio_set_level(PWR_EN_595, 1);
#define CLOCK_HIGH                  gpio_set_level(STCP, 1)
#define CLOCK_LOW                   gpio_set_level(STCP, 0)
#define latch_HIGH                  gpio_set_level(SHCP, 1)
#define latch_LOW                   gpio_set_level(SHCP, 0)
#define CLOCK_HIGH_1                gpio_set_level(STCP1,1)
#define CLOCK_LOW_1                 gpio_set_level(STCP1,0)
#define DS1_LOW                     gpio_set_level(DS,0)

// SWITCH
#define ROW1_LOW                    gpio_set_level(SWITCH_A,0)
#define ROW2_LOW                    gpio_set_level(SWITCH_B,0)
#define ROW3_LOW                    gpio_set_level(SWITCH_C,0)
#define ROW4_LOW                    gpio_set_level(SWITCH_D,0)
#define ROW1_HIGH                   gpio_set_level(SWITCH_A,1)
#define ROW2_HIGH                   gpio_set_level(SWITCH_B,1)
#define ROW3_HIGH                   gpio_set_level(SWITCH_C,1)
#define ROW4_HIGH                   gpio_set_level(SWITCH_D,1)



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "sys/time.h"
#include "nvs.h"
#include "esp_mac.h"
/****************************/
#include "driver/i2s_std.h"
/****************************/
#include "esp_flash_partitions.h"
#include "esp_app_format.h"
#include "esp_partition.h"
#include "esp_event.h"
/****************************/
#include "soc/uart_struct.h"
#include "esp_task_wdt.h"
#include "esp_adc_cal.h"
#include "esp_ota_ops.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_littlefs.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
/******** Bluetooth ********/
#include "esp_gatt_common_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_log.h"
#include "esp_bt.h"

/******** Freertos********/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/****************************/
#include "driver/timer.h"
#include "driver/twai.h"
#include "driver/uart.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"
#include "esp_rom_gpio.h" 
/****************************/

#include "global.h"
#include "cpu.h"
#include "led.h"
#include "HC595.h"
#include "ble.h"
#include "key.h"
#include "accelerometer.h"
#include "rtc.h"
#include "AHT.h"
#include "lcd.h"
#include "serial_flash.h"
#include "OTA.h"
#include "sound.h"
#endif