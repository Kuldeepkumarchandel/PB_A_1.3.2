/*
 * accelerometer.h
 *
 * Created: 25-Apr-16 12:57:45 PM
 *  Author: lenovo
 */ 


#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_
#include "header.h"

#define ACC_SDA     SDAA
#define ACC_SCL     SCLL


#define SCLSET      gpio_set_level(ACC_SCL, 1);
#define SCLCLR      gpio_set_level(ACC_SCL, 0);

#define SCL_OUTPUT 	esp_rom_gpio_pad_select_gpio(ACC_SCL);    gpio_set_direction(ACC_SCL, GPIO_MODE_OUTPUT);   
#define SDA_OUTPUT  esp_rom_gpio_pad_select_gpio(ACC_SDA);    gpio_set_direction(ACC_SDA, GPIO_MODE_OUTPUT);
#define SDA_INPUT	esp_rom_gpio_pad_select_gpio(ACC_SDA);    gpio_set_direction(ACC_SDA, GPIO_MODE_INPUT); gpio_pullup_en(ACC_SDA);

#define ACC_SDASET  gpio_set_level(ACC_SDA, 1);
#define ACC_SDACLR  gpio_set_level(ACC_SDA, 0);

#define ACC_ADDR                0x32
#define STATUS_REG_AUX			0x07
#define OUT_ADC1_L				0x08
#define OUT_ADC1_H				0x09
#define OUT_ADC2_L				0x0A
#define OUT_ADC2_H				0x0B
#define OUT_ADC3_L				0x0C
#define OUT_ADC3_H				0x0D
#define INT_COUNTER_REG			0x0E
#define WHO_AM_I				0x0F
#define TEMP_CFG_REG			0x1F
#define CTRL_REG1				0x20
#define CTRL_REG2				0x21
#define CTRL_REG3				0x22
#define CTRL_REG4				0x23
#define CTRL_REG5				0x24
#define CTRL_REG6				0x25
#define REFERENCE				0x26
#define STATUS_REG2				0x27
#define OUT_X_L					0x28
#define OUT_X_H					0x29
#define OUT_Y_L					0x2A
#define OUT_Y_H					0x2B
#define OUT_Z_L					0x2C
#define OUT_Z_H					0x2D
#define FIFO_CTRL_REG			0x2E
#define FIFO_SRC_REG			0x2F
#define INT1_CFG				0x30
#define INT1_SOURCE 			0x31
#define INT1_THS				0x32
#define INT1_DURATION			0x33
#define CLICK_CFG				0x38
#define CLICK_SRC				0x39
#define CLICK_THS				0x3A
#define TIME_LIMIT				0x3B
#define TIME_LATENCY 			0x3C
#define TIME_WINDOW 			0x3D

void IRAM_ATTR acc_isr_handler(void* arg);
void IRAM_ATTR acc2_isr_handler(void* arg);
void delay_us(int time);
char acc_write(char skip ,char device_add, char reg_add, char data);
char acc_read(char skip,char device_add,char reg_add);
char tx(char x);
void start(void);
void stop(void);
char rx(void);
void accelerometer_init(void);
void acc_tilt_detection(void);
void read_z_zxis_acceleration(void);

#endif /* ACCELEROMETER_H_ */
