#include "header.h"

// 74HC595

void clock_enable_2(void)
{
    CLOCK_LOW;
    delay_us(5); 
    CLOCK_HIGH; 
    delay_us(5); 
  
}

void clock_enable_1(void)
{
    CLOCK_LOW_1;
    delay_us(5); 
    CLOCK_HIGH_1; 
    delay_us(5); 
  
}

void latch_enable(void)
{
    latch_LOW;
    delay_us(5); 
    latch_HIGH;
    delay_us(5); 
}

void Disable_595_power()
{
    PWR_EN_595_OFF;CLOCK_LOW;latch_LOW;CLOCK_LOW_1;DS1_LOW;LCD_SDACLR;LCD_SCLCLR;
}

/*
*
*   DS1   --- IC number 1
*   DS2   --- IC number 2
*
*/

void send_data_on_595(uint8_t data_out, uint8_t IC_number)
{
    int i, DS_PIN;
    // printf("data_out %d\t",data_out )  ; 
    if(IC_number == HIGH_SIDE_U11)
    {
        for (i=0 ; i<8 ; i++)
        {
            DS_PIN = data_out & ((0x80)>>i);
            gpio_set_level(DS, DS_PIN);
            latch_enable(); 
        }
        clock_enable_1();
    }
    else //LOW_SIDE_U8
    {
        for (i=0 ; i<8 ; i++)
        {
            if((data_out& ((0x80)>>i)) != 0x00) 
            {
                gpio_set_level(DS, 0);
            }
            else
            {
                gpio_set_level(DS, 1);
            }  
            latch_enable(); 
        }
        clock_enable_2();
    }
}






