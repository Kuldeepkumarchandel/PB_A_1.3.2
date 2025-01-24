#ifndef _74HC595_h_
#define _74HC595_h_

void latch_enable(void);
void clock_enable_1(void);
void clock_enable_2(void);
void send_data_on_595(uint8_t data_out, uint8_t IC_number);
void Disable_595_power();
#endif