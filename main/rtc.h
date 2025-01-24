
#ifndef RTC_H
#define RTC_H
#include "header.h"
#include "global.h"


extern char g_day[4]; 
extern char g_hour[5]; 
extern char g_min[5]; 
extern char g_date[15];

#define HT1380_SDA          RTC_SDA
#define HT1380_SCL          RTC_SCL

#define HT1380_RST_HIGH     gpio_set_level(RTC_RESET, 1)
#define HT1380_IO_HIGH      gpio_set_level(HT1380_SDA, 1) 
#define HT1380_CLK_HIGH     gpio_set_level(HT1380_SCL, 1) 
#define HT1380_RST_LOW      gpio_set_level(RTC_RESET, 0) 
#define HT1380_IO_LOW       gpio_set_level(HT1380_SDA, 0)
#define HT1380_CLK_LOW      gpio_set_level(HT1380_SCL, 0)
#define HT1380_IO_TO_INPUT  esp_rom_gpio_pad_select_gpio(HT1380_SDA); gpio_set_direction(HT1380_SDA, GPIO_MODE_INPUT)
#define HT1380_IO_TO_OUTPUT esp_rom_gpio_pad_select_gpio(HT1380_SDA); gpio_set_direction(HT1380_SDA, GPIO_MODE_OUTPUT)
#define HT1380_IO_INPUT     gpio_get_level(HT1380_SDA)

void rtcc_gpio_init();
extern void update_time(void);
extern void write_rtc_burst_mode(unsigned char sec,unsigned char min,unsigned char hh,unsigned char date,unsigned char mon,unsigned char day,unsigned char year,char wp);
extern void read_rtc_burst_mode(unsigned char *sec,unsigned char *min,unsigned char *hh,unsigned char *date,unsigned char *mon,unsigned char *day,unsigned char *year,unsigned char *wp);
extern char read_clk(void);
extern char test_time_validity(unsigned char ss,unsigned char mm,unsigned char hh,unsigned char date,unsigned char mon,unsigned char day,unsigned char year);
extern void reset_rtc(char mode);
//extern void set_time(char ss,char mm,char hh);
void set_time(char day, char ss,char mm,char hh,char date,char mon,char year);
extern void enable_osc_dis_wp(void);
extern char I2C_write(unsigned char dat);
extern void I2C_clock(void);
extern void I2C_delay(void);
void decode_day(uint8_t l_day);
int isLeapYear(int year);
uint8_t Read_clock_and_compare_with_alarm_epoch(char mode);
void convertIdToRowColumn_GlowLedMatrix(int id,char glow_type);
void Get_next_alarm_and_Show_on_display(uint8_t LP,int32_t next_time_epoch);
void get_current_alarm_number(char* alarm_Number);
char READ_RTC_and_GET_Epoch(uint32_t *epoch);
void get_week_number(char* weekNumber);
int32_t Find_next_alarm_and_current_epoch(char *current_alarm, char *next_alarm, uint32_t *current_epoch,uint16_t* next_compartment);
char getWeek(time_t timestamp);
#endif
