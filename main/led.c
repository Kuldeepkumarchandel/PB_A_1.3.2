#include "header.h"
uint8_t led_matrix[8] = {0x00},led_toggle_matrix[8] = {0x00};

void LED28_Control(int column,int row,char value)
{	
	if((value == 0) || (value == 1))
	{
		f.g_led_buffer_busy_F  = 1;
		if(value == 1)
		{
			led_matrix[column] |=  (0x01<<row);
		}
		else
		{
			led_matrix[column] &=  ~(0x01<<row);
		}
		
	}
	else if(value == 2)
	{
		led_toggle_matrix[column] |=  (0x01<<row);
		g_led_toggle_cnt=0;g_led_toggle_state=1;g_led_toggle_threshold=100;
	}
	else if(value == 3)
	{
		led_toggle_matrix[column] &=  ~(0x01<<row);
		led_matrix[column] &=  ~(0x01<<row);
	}
	f.g_led_buffer_busy_F  = 0;
}

void RGB_Buzzer_Control(int8_t RED,int8_t GREEN,int8_t BLUE,int8_t Buzzer)
{
	f.g_led_buffer_busy_F  = 1;
	for(int column=0;column<1;column++)// if you want to increase brightness of RGB then increase 1 to 2,3,4 up to 8
	{
		if(RED == 1)
		{
			led_matrix[column] |=  (0x01<<RGB_Red_LED);
		}
		else if(RED == 0)
		{
			led_matrix[column] &=  ~(0x01<<RGB_Red_LED);
		}
		else if(RED == 2)
		{
			led_toggle_matrix[column] |=  (0x01<<RGB_Red_LED);
			g_led_toggle_cnt=0;g_led_toggle_state=1;g_led_toggle_threshold=100;
		}

		if(GREEN == 1)
		{
			led_matrix[column] |=  (0x01<<RGB_Green_LED);
		}
		else if(GREEN == 0)
		{
			led_matrix[column] &=  ~(0x01<<RGB_Green_LED);
		}
		else if(GREEN == 2)
		{
			led_toggle_matrix[column] |=  (0x01<<RGB_Green_LED);
			g_led_toggle_cnt=0;g_led_toggle_state=1;g_led_toggle_threshold=100;
		}

		if(BLUE == 1)
		{
			led_matrix[column] |=  (0x01<<RGB_Blue_LED);
		}
		else if(BLUE == 0)
		{
			led_matrix[column] &=  ~(0x01<<RGB_Blue_LED);
		}	
		else if(BLUE == 2)
		{
			led_toggle_matrix[column] |=  (0x01<<RGB_Blue_LED);
			g_led_toggle_cnt=0;g_led_toggle_state=1;g_led_toggle_threshold=100;
		}
		//f.g_buzzer_ON = Buzzer;
	}
	f.g_led_buffer_busy_F  = 0;
}

/**
 * The function Run_LED_Matrix takes an array of characters representing an LED matrix and displays it
 * on the LED matrix.
 * 
 * @param led_matrix The parameter "led_matrix" is a pointer to a character array that represents the
 * LED matrix. Each element in the array represents a row of the LED matrix, and the value of each
 * element represents the state of the LEDs in that row.
 */
void Run_LED_Matrix(char* led_matrix)
{		
	char temp=0;
	int i=0;
	for(i=0;i<8;i++)
		{
			if(led_matrix[i]!=0)
			{
				break;
			}
		}
	
	// if(i<8)
	{
		PWR_EN_595_ON;
			for(i=0;i<8;i++)
			{
				temp = led_matrix[i];
				// if(temp!=0)
				// {
					if(f.g_audio_ON == 1)
					{
						send_data_on_595(0b00000000|0x01,LOW_SIDE_595);//Reset 
					}
					else
					{
						send_data_on_595(0b00000000,LOW_SIDE_595);//Reset 
					}
				//}
				if(f.g_buzzer_ON == 1 || f.g_buzzer_independent_ON == 1)
				{
					send_data_on_595((0x01<<i)|0x01,HIGH_SIDE_595);
				}
				else
				{
					send_data_on_595((0x01<<i) & (~0x01),HIGH_SIDE_595);
				}
				if(f.g_audio_ON == 1)
				{
					send_data_on_595((char)temp|0x01,LOW_SIDE_595);
				}
				else
				{
					send_data_on_595((char)temp,LOW_SIDE_595);
				}
				//vTaskDelay(10/portTICK_PERIOD_MS);
				esp_rom_delay_us(1500);
			}
			if(f.g_audio_ON == 1)
			{
				send_data_on_595(0b00000000|0x01,LOW_SIDE_595);//Reset 
			}
			else
			{
				send_data_on_595(0b00000000,LOW_SIDE_595);//Reset 
			}
		//Disable_595_power();
	}
}

void wellcome_greetings()
{
    RGB_Buzzer_Control(1,0,0,0);LED28_Control(MON,MORNING,1);LED28_Control(MON,AFTERNOON,1);LED28_Control(MON,EVENING,1);LED28_Control(MON,NIGHT,1);LED_MATRIX_RESET();
 
    RGB_Buzzer_Control(0,1,0,1);LED28_Control(TUE,MORNING,1);LED28_Control(TUE,AFTERNOON,1);LED28_Control(TUE,EVENING,1);LED28_Control(TUE,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,0,1,0);LED28_Control(WED,MORNING,1);LED28_Control(WED,AFTERNOON,1);LED28_Control(WED,EVENING,1);LED28_Control(WED,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(1,0,0,1);LED28_Control(THU,MORNING,1);LED28_Control(THU,AFTERNOON,1);LED28_Control(THU,EVENING,1);LED28_Control(THU,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,1,0,0);LED28_Control(FRI,MORNING,1);LED28_Control(FRI,AFTERNOON,1);LED28_Control(FRI,EVENING,1);LED28_Control(FRI,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,0,1,1);LED28_Control(SAT,MORNING,1);LED28_Control(SAT,AFTERNOON,1);LED28_Control(SAT,EVENING,1);LED28_Control(SAT,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,0,0,1);LED28_Control(SUN,MORNING,1);LED28_Control(SUN,AFTERNOON,1);LED28_Control(SUN,EVENING,1);LED28_Control(SUN,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();


	RGB_Buzzer_Control(1,0,0,0);LED28_Control(MON,MORNING,1);LED28_Control(TUE,MORNING,1);LED28_Control(WED,MORNING,1);LED28_Control(THU,MORNING,1);LED28_Control(FRI,MORNING,1);LED28_Control(SAT,MORNING,1);LED28_Control(SUN,MORNING,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,1,0,1);LED28_Control(MON,AFTERNOON,1);LED28_Control(TUE,AFTERNOON,1);LED28_Control(WED,AFTERNOON,1);LED28_Control(THU,AFTERNOON,1);LED28_Control(FRI,AFTERNOON,1);LED28_Control(SAT,AFTERNOON,1);LED28_Control(SUN,AFTERNOON,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,0,1,0);LED28_Control(MON,EVENING,1);LED28_Control(TUE,EVENING,1);LED28_Control(WED,EVENING,1);LED28_Control(THU,EVENING,1);LED28_Control(FRI,EVENING,1);LED28_Control(SAT,EVENING,1);LED28_Control(SUN,EVENING,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,0,0,1);LED28_Control(MON,NIGHT,1);LED28_Control(TUE,NIGHT,1);LED28_Control(WED,NIGHT,1);LED28_Control(THU,NIGHT,1);LED28_Control(FRI,NIGHT,1);LED28_Control(SAT,NIGHT,1);LED28_Control(SUN,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(1,0,0,0);LED28_Control(MON,MORNING,1);LED28_Control(TUE,MORNING,1);LED28_Control(WED,MORNING,1);LED28_Control(THU,MORNING,1);LED28_Control(FRI,MORNING,1);LED28_Control(SAT,MORNING,1);LED28_Control(SUN,MORNING,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,1,0,1);LED28_Control(MON,AFTERNOON,1);LED28_Control(TUE,AFTERNOON,1);LED28_Control(WED,AFTERNOON,1);LED28_Control(THU,AFTERNOON,1);LED28_Control(FRI,AFTERNOON,1);LED28_Control(SAT,AFTERNOON,1);LED28_Control(SUN,AFTERNOON,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,0,1,0);LED28_Control(MON,EVENING,1);LED28_Control(TUE,EVENING,1);LED28_Control(WED,EVENING,1);LED28_Control(THU,EVENING,1);LED28_Control(FRI,EVENING,1);LED28_Control(SAT,EVENING,1);LED28_Control(SUN,EVENING,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();
	RGB_Buzzer_Control(0,0,0,1);LED28_Control(MON,NIGHT,1);LED28_Control(TUE,NIGHT,1);LED28_Control(WED,NIGHT,1);LED28_Control(THU,NIGHT,1);LED28_Control(FRI,NIGHT,1);LED28_Control(SAT,NIGHT,1);LED28_Control(SUN,NIGHT,1);vTaskDelay(200/portTICK_PERIOD_MS);  LED_MATRIX_RESET();

    LED_MATRIX_SET();vTaskDelay(500/portTICK_PERIOD_MS);
	LED_MATRIX_RESET();vTaskDelay(500/portTICK_PERIOD_MS);
    LED_MATRIX_SET();vTaskDelay(500/portTICK_PERIOD_MS);
	LED_MATRIX_RESET();vTaskDelay(500/portTICK_PERIOD_MS);

}

void LED_MATRIX_RESET()
{
	memset((char*)led_matrix, 0x00, sizeof(led_matrix));
}

void LED_MATRIX_SET()
{
	memset((char*)led_matrix, 0xFF, sizeof(led_matrix));
}

void led_task_manager()
{
	uint16_t delay = 0;
	char l_led_matrix_buff[8]={0};
    while(1)
    {
			
        if(f.g_led_buffer_busy_F == 0)
        {
            memcpy(l_led_matrix_buff,led_matrix,sizeof(led_matrix));
        }
        Run_LED_Matrix((char*)&l_led_matrix_buff);
		Run_Buzzer(); 		
        vTaskDelay(10/portTICK_PERIOD_MS);
		delay += 10;
		if(delay >= 100)
		{
			static char state;
			delay = 0;
			if(state == 0)
			{
				state = 1;
			}
			else if(state == 1)
			{
				state = 0;
			}
			if(g_led_toggle_state == 2)  
			{
				state = 0;
			}  
			for(int col=0;col<8;col++)
			{
				for(int row=0;row<8;row++)
				{					
					if((led_toggle_matrix[col]&(0x01<<row)) != 0)
					{
						LED28_Control(col,row,state);	
					}									
				}
			}
			if(g_led_toggle_state == 2)  
			{
				g_led_toggle_state = 0;
				memset((char*)led_toggle_matrix,0x00,sizeof(led_toggle_matrix));
			}  	
		}
    }
}

/**
 * @ NOTE : We need to remember that the buzzer will "ON" half of our calculation  
*/
void Blow_Buzzer(int off_on_interval_ms, int timeout_in_ms)
{
	if(timeout_in_ms > 100)
	{
		g_Buzzer_Threshold = (timeout_in_ms/100);
	}
	else
	{
		g_Buzzer_Threshold = 1;
	}
	g_Buzzer_toggle_interval	 = off_on_interval_ms;
}

void play_panic_buzzer(void)
{
	static char state;
	state++;
	if(state < 3)
	{
		RGB_Buzzer_Control(1,0,0,0);  
		f.g_buzzer_independent_ON=1;
		vTaskDelay(100/portTICK_PERIOD_MS);
		f.g_buzzer_independent_ON=0;
		vTaskDelay(100/portTICK_PERIOD_MS);
		RGB_Buzzer_Control(0,0,0,0);  
	}
	else if((state >= 3) && (state < 6))
	{
		RGB_Buzzer_Control(1,0,0,0); 
		f.g_buzzer_independent_ON=1;
		vTaskDelay(300/portTICK_PERIOD_MS);
		f.g_buzzer_independent_ON=0;
		vTaskDelay(100/portTICK_PERIOD_MS);
		RGB_Buzzer_Control(0,0,0,0);  
	}
	else if((state >= 6) && (state < 9))
	{
		RGB_Buzzer_Control(1,0,0,0); 
		f.g_buzzer_independent_ON=1;
		vTaskDelay(100/portTICK_PERIOD_MS);
		f.g_buzzer_independent_ON=0;
		vTaskDelay(100/portTICK_PERIOD_MS);
		RGB_Buzzer_Control(0,0,0,0);  
	}
	else
	{
		state = 0;
	}
}
void Stop_Buzzer()
{
	g_Buzzer_Threshold = 0;
	g_Buzzer_toggle_interval	 = 0;
	f.g_buzzer_ON = 0;
}
void Run_Buzzer()
{
	static int cnt;
	static char flag;
	if(g_Buzzer_toggle_interval != 0)
	{
		cnt += 20;
		if(cnt >= g_Buzzer_toggle_interval)
		{
			cnt = 0;
			if(flag == 0)
			{
				flag = 1;
				f.g_buzzer_ON=0;
				RGB_Buzzer_Control(0,0,0,0);
			}
			else
			{
				flag = 0;
				f.g_buzzer_ON=1;
				RGB_Buzzer_Control(1,0,0,0);
			}
		}
	}
	else if(f.g_buzzer_ON == 1)
	{
		flag=0;
		cnt=0;
		f.g_buzzer_ON = 0;
		RGB_Buzzer_Control(0,0,0,0);
	}
}


void led_task_init()
{
	xTaskCreate(led_task_manager, "led_task_manager",2*1024, 0, 5, NULL);
}