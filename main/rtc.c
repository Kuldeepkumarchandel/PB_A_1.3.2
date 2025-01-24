#define rtc_c
#include "header.h"

char g_day[4] = {0}; 
char g_hour[5] = {0}; 
char g_min[5] = {0}; 
char g_sec[5] = {0}; 
char g_date[15] = {0}; 
extern char total_53weeks_available;




void _delay_us(int a)
{
	esp_rom_delay_us(a);
}

void _delay_ms(int a)
{
	vTaskDelay((a+1) / portTICK_PERIOD_MS);
}

void update_time(void)
{
	/*if(f.g_flag == (char)0x01)
	{		
	    handle_led();
			
			char valid=0;
			valid = read_clk();
				if(valid!='V')
				{f.g_error_rtc_sense_F=1;}	
				else
				{
				f.g_error_rtc_sense_F=0;						
				  static char  mm;
				  if(g_minute!=mm)
				  {
				  f.g_frquent_off_on_protection_F = 0;
					mm=g_minute;
				  }
					if((g_system_state == DEFAULT_STATE) || (g_system_state == STATUS_STATE))
					{	decision_making();}
				  
				}
		
		f.g_flag = 0;
	}*/
}

void write_rtc_burst_mode(unsigned char sec,unsigned char min,unsigned char hh,unsigned char date,unsigned char mon,unsigned char day,unsigned char year,char wp)
{
	hh = hh & 0x3f;
	unsigned char temp[8]={sec,min,hh,date,mon,day,year,0x00};
	int i,k;
	unsigned char Cmd = 0xbe;
	//printf("\n befor  write burst rtc.hh = %d,rtc.mm = %d rtc.ss =%d\t rtc.date = %d,rtc.mon = %d rtc.year =%d\n",hh,min,sec,date,mon,year);
	HT1380_IO_TO_OUTPUT;
	reset_rtc(1);
		for(i=0;i<8;i++)
		{
			if((Cmd & (char)(((char)0x01)<<i)) != 0x00)
			{
				HT1380_IO_HIGH;
			}
			else
			{
				HT1380_IO_LOW;
			}
			I2C_clock();
		}

		for(i=0;i<8;i++)
		{
			for(k=0;k<8;k++)
			{
				if((temp[i] & (char)(((char)0x01)<<k)) != 0)
				{
					HT1380_IO_HIGH;
				}
				else
				{
					HT1380_IO_LOW;
				}
		
				I2C_clock();
			}
		}
	reset_rtc(0);
}
		
void read_rtc_burst_mode(unsigned char *sec,unsigned char *min,unsigned char *hh,unsigned char *date,unsigned char *mon,unsigned char *day,unsigned char *year,unsigned char *wp)
{
	int i=0,k=0;
	unsigned char temp[8]={0};	
	unsigned char Cmd = 0xbf;
	memset(temp,0x00,8);
	HT1380_IO_TO_OUTPUT;
	reset_rtc(1);
		for(i=0;i<8;i++)
		{		
		
				if((Cmd & (char)(((char)0x01)<<i))!= 0x00)
				{
					HT1380_IO_HIGH;
				}
				else
				{
					HT1380_IO_LOW;
				}
		
			I2C_clock();
		}	
	HT1380_IO_TO_INPUT;
	I2C_delay();
		for(i=0;i<8;i++)
		{
			for(k=0;k<8;k++)
			{
				HT1380_CLK_HIGH;
				I2C_delay();
				HT1380_CLK_LOW;
				I2C_delay();
				temp[i] = temp[i] >> 1;			
				if((HT1380_IO_INPUT) != (char)0x00)
				{
					temp[i] = temp[i] | 0x80;
				}
			}
		}
	reset_rtc(0);	
	*sec = temp[0];*min = temp[1];*hh = temp[2];*date = temp[3];*mon = temp[4];*day = temp[5];*year = temp[6];
}

void decode_day(uint8_t l_day)
{
	memset((char*)&g_day,0x00,3);
	switch (l_day)
	{
		case 0:
			memcpy((char*)&g_day,"SUN",3);
			break;
		case 1:
			memcpy((char*)&g_day,"MON",3);
			break;
		case 2:
			memcpy((char*)&g_day,"TUE",3);
			break;
		case 3:
			memcpy((char*)&g_day,"WED",3);
			break;
		case 4:
			memcpy((char*)&g_day,"THU",3);
			break;
		case 5:
			memcpy((char*)&g_day,"FRI",3);
			break;
		case 6:
			memcpy((char*)&g_day,"SAT",3);
			break;
		case 7:
			memcpy((char*)&g_day,"SUN",3);
			break;
		
		default:
			break;
	}
}

char read_clk(void)
{
	char valid=0;
	int i=0;
	unsigned char ss,mm,hh,date,mon,day,year,wp;
	for(i=0;i<10;i++)
	{
		read_rtc_burst_mode(&ss,&mm,&hh,&date,&mon,&day,&year,&wp);
		// printf("\n after burst rtc.hh = %x,rtc.mm = %x rtc.ss =%x\t rtc.date = %x,rtc.mon = %x rtc.year =%x\n",hh,mm,ss,date,mon,year);
		valid = test_time_validity(ss,mm,hh,date,mon,day,year);
		// printf("\n iggfgalidation rtc.hh = %d,rtc.mm = %d rtc.ss =%d\t rtc.date = %d,rtc.mon = %d rtc.year =%d\n",hh,mm,ss,date,mon,year);
		
		if(valid == 'V')break;
		_delay_ms(200);	 
	}
	// if(ss==3)return(-1);//vvvvvv
	// printf("\n ikkkktion rtc.hh = %d,rtc.mm = %d rtc.ss =%d\t rtc.date = %d,rtc.mon = %d rtc.year =%d\n",hh,mm,ss,date,mon,year);
	return(valid);		  	 
}

char test_time_validity(unsigned char ss,unsigned char mm,unsigned char hh,unsigned char date,unsigned char mon,unsigned char day,unsigned char year)
{
  	 char valid;  
	 hh = ((hh&0x30)>>4)*10 + (hh&0x0F);
	 mm = ((mm&0x70)>>4)*10 + (mm&0x0F);
	 ss = ((ss&0x70)>>4)*10 + (ss&0x0F);
	 date = ((date&0x30)>>4)*10 + (date&0x0F);
	 mon = ((mon&0x10)>>4)*10 + (mon&0x0F);
	 day = (day&0x0F);
	 year = ((year&0xF0)>>4)*10 + (year&0x0F);

		//   printf("\n inside validation rtc.hh = %d,rtc.mm = %d rtc.ss =%d\t rtc.date = %d,rtc.mon = %d, rtc,day %d, rtc.year =%d\n",hh,mm,ss,date,mon,day,year);
	    if((hh <= 23) && ( mm <= 59) && (ss <= 59) && ((date>=1)&&(date<=31)) && ((mon>=1)&&(mon<=12)) && ((day>=1)&&(day<=7)) && ((year>=1)&&(year<=99)))
	    {
			if(isLeapYear((2000+year)))
			{
				total_53weeks_available = 1;
			}
			else
			{
				total_53weeks_available = 0;
			}
			memset((char*)&g_date,0x00,sizeof(g_date));
			sprintf((char*)g_date, "%02d%02d%02d",date,mon,year);
			decode_day(day);
			memset((char*)&g_hour,0x00,sizeof(g_hour));
			sprintf((char*)g_hour, "%02d",hh);
			memset((char*)&g_min,0x00,sizeof(g_min));
			sprintf((char*)g_min, "%02d",mm);
			memset((char*)&g_sec,0x00,sizeof(g_sec));
			sprintf((char*)g_sec, "%02d",ss);
			// g_timee[0]=hh;g_timee[1]=mm;g_timee[2]=ss;g_timee[3]=0;
			// ESP_LOGI("RTCCCCC"," rtc.hh = %d,rtc.mm = %d rtc.ss =%d\t rtc.date = %d,rtc.mon = %d rtc.year =%d\n",hh,mm,ss,date,mon,year);
	         valid = 'V';
	    }
	    else
	    {
	        valid = 0x00;
	    }

     return(valid);

}

void reset_rtc(char mode)
  {
	  if(mode == 1)
	  {
		 HT1380_CLK_LOW;
		 HT1380_RST_LOW;
		 I2C_delay();
		 HT1380_RST_HIGH;
		 I2C_delay();
	  }
	  else
	  {
		 HT1380_CLK_LOW;
		 HT1380_RST_LOW;
		 HT1380_RST_LOW;
		 I2C_delay();
	  }
	 
  }

void set_time(char day, char ss,char mm,char hh,char date,char mon,char year)
  {
	  enable_osc_dis_wp();
	//   printf("\n WRITE hh = %d,mm = %d ss =%d\n",hh,mm,ss);
	  int i=0;
	  for(i=0;i<30;i++)
	  {_delay_ms(100);}
	  write_rtc_burst_mode(ss,mm,hh,date,mon,day,year,0x00);
	  //f.g_flag = 1;	 	 
  }

void enable_osc_dis_wp(void)
  {
	  reset_rtc(1);
	  I2C_write(0x8E);
	  I2C_write(0x80);	 
	  reset_rtc(1);
	  I2C_write(0x80);
	  I2C_write(0x00);
	  reset_rtc(0);	  
  }
 
char I2C_write(unsigned char dat)
 {
	 char data_bit = 0;
	 unsigned char i;
	 HT1380_IO_TO_OUTPUT;
	 I2C_delay();
	 for(i=0;i<8;i++)
	 {
		 data_bit = dat & 0x01; 
			 if(data_bit != 0x00)
			 {
				 HT1380_IO_HIGH;
			 }
			 else
			 {
				 HT1380_IO_LOW;
			 }		 
		 I2C_clock();

		 dat = dat>>1;
	 }

	 HT1380_IO_HIGH; 
     I2C_clock();
	 HT1380_IO_TO_INPUT;
	 I2C_delay();
	 data_bit = HT1380_IO_INPUT;
	

	 I2C_delay();

	 return data_bit; 
 }

void I2C_clock(void)
 {
	I2C_delay();
	HT1380_CLK_LOW;  /* Clear CLK */
	I2C_delay();
    HT1380_CLK_HIGH;/* Start clock */
	I2C_delay();
 }

void I2C_delay(void)
 {
	 _delay_us(10);
 }
 
void rtcc_gpio_init()
{
	esp_rom_gpio_pad_select_gpio(HT1380_SCL); gpio_set_direction(HT1380_SCL, GPIO_MODE_OUTPUT); //scl
	esp_rom_gpio_pad_select_gpio(RTC_RESET);  gpio_set_direction(RTC_RESET, GPIO_MODE_OUTPUT);  //rst
}

/*unsigned char I2C_read(void)
 {
	 char rd_bit;
	 unsigned char i, dat;
	 HT1380_IO_TO_OUTPUT;
	 dat = 0x00;

	 for(i=0;i<8;i++) 
	 {
		 I2C_delay();
		 HT1380_CLK_HIGH; 
		 I2C_delay();
		 HT1380_IO_TO_INPUT;
		 I2C_delay();
		 rd_bit = HT1380_IO_INPUT; 
		 dat = dat>>1;
		 if(rd_bit != 0x00)
		 {
			 dat = dat | 0x80; 
		 }
		 
		 HT1380_CLK_LOW;
	 }

	 return dat;
 }	*/


int isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void Get_next_alarm_and_Show_on_display(uint8_t LP,int32_t next_time_epoch )
{
	char l_min[5]           = {0};
	char l_hour[5]          = {0};
	time_t now = next_time_epoch;//eeprom.config_alarm_time[LP].timer_epoch;
	struct tm timeinfo;
	localtime_r(&now, &timeinfo);
	memset((char*)&l_hour,0x00,sizeof(l_hour));
	sprintf((char*)l_hour, "%02d",timeinfo.tm_hour);
	memset((char*)&l_min,0x00,sizeof(l_min));
	sprintf((char*)l_min, "%02d",timeinfo.tm_min);
	if(eeprom.time_mode !=TIME_MODE_24HR)
	{
		if(atoi(l_hour) > 12)
		{
			memset((char*)&l_hour,0x00,sizeof(l_hour));
			sprintf((char*)l_hour, "%02d",((char)atoi(l_hour) - 12));
			show_alarm_time((char *)&l_hour, (char *)&l_min,TIME_MODE_PM);
		}
		else	show_alarm_time((char *)&l_hour, (char *)&l_min, TIME_MODE_AM);
	}
	else
	{
		show_alarm_time((char *)&l_hour, (char *)&l_min,TIME_MODE_24HR);
	}
	sprintf(l_hour,"%02d",(LP));   // to show as in app  
	ESP_LOGI("PBbbbbbbbbbbbbbbbbbbbb","boxxxxxxiddddd %s: %d",l_hour, LP);
	write_BoxId(l_hour);	
}

void get_current_alarm_number(char* alarm_Number)
{
	// will need to discuss these points
}

char getWeek(time_t timestamp)
{
    struct tm target;
    localtime_r(&timestamp, &target);

    int dayNr = (target.tm_wday + 6) % 7;
    target.tm_mday -= dayNr - 3;
    mktime(&target);

    long firstThursday = mktime(&target);

    target.tm_mon = 0;
    target.tm_mday = 1;
    mktime(&target);

    if (target.tm_wday != 4) {
        target.tm_mday += (4 - target.tm_wday + 7) % 7;
        mktime(&target);
    }
    return 1 + ((firstThursday - mktime(&target)) / 604800);
}

void get_week_number(char* weekNumber)
{
	static char WeekNumber = 0;
	char temp[10] = {0};
	static char l_date[15] = {0};
	if(memcmp(g_date, l_date, sizeof(l_date)) != 0)
	{
		memcpy(l_date, g_date, sizeof(l_date));
		sprintf(temp,"%x%x%x",atoi(g_hour),atoi(g_min),atoi(g_sec));
		uint32_t l_epoch = convert_time_date_into_epoch((char*)&g_date,(char*)&temp);
		WeekNumber = getWeek((time_t)l_epoch);
	}
	sprintf(weekNumber,"%2d",WeekNumber);
	// ESP_LOGI("PB","week number %d",WeekNumber);
}

char READ_RTC_and_GET_Epoch(uint32_t *epoch)
{
	static char ret = 0;
	static char temp[10] = {0};
	ret = read_clk();
	if(ret == 'V')
	{
		// ESP_LOGI("RTCCCCC","hh = %s, len %d,mm = %s, len %d \tdate = %s, len %d, ret = %c\n",g_hour,strlen(g_hour),g_min,strlen(g_min),g_date,strlen(g_date), ret);
		if(eeprom.time_mode !=TIME_MODE_24HR)
		{
			if(atoi(g_hour) > 12)
			{
				memset((char*)&temp,0x00,sizeof(temp));
				sprintf((char*)temp, "%02d",((char)atoi(g_hour) - 12));
				show_rtc_time((char*)&g_day,(char*)&temp,(char*)&g_min, TIME_MODE_PM);
			}
			else
			{
				show_rtc_time((char*)&g_day,(char*)&g_hour,(char*)&g_min, TIME_MODE_AM);
			}
		}
		else
		{
			show_rtc_time((char*)&g_day,(char*)&g_hour,(char*)&g_min, TIME_MODE_24HR);
		}
		show_date((char*)&g_date);
		sprintf(temp,"%02d%02d%02d",atoi(g_hour),atoi(g_min),atoi(g_sec));
		*epoch = convert_time_date_into_epoch((char*)&g_date,(char*)&temp);
		return ret;
	}
	return 0;
}


uint8_t Read_clock_and_compare_with_alarm_epoch(char mode)
{

	char current_alarm[MAXIMUM_SUPPORTED_TIMER] = {0}, next_alarm = 0; uint32_t l_epoch = 0;uint16_t next_compartment=0;		
	/*Get All Timer_alarm lies between the epoch window, if not a single timer lies in the epoch windoe then find the next nearest timer LP, and current epoch value*/
	int32_t ret = Find_next_alarm_and_current_epoch((char*)&current_alarm[0], (char*)&next_alarm, &l_epoch,&next_compartment);
	ESP_LOGI("NEXT ALARM","%d, ret %ld", next_alarm, ret);
	if(ret != -1)
	{	
		if(mode == 0)
		{
			if(ret<5)
			{
			    return(1);
			}
			else
			{
				return(0);
			}
		}
		for(int i = 0; i < eeprom.total_timers_configured; i++)
		{
			ESP_LOGE("next","current_alarm[%d] : %d",i, current_alarm[i]);
			if(current_alarm[i] == 1)
			{
				if(eeprom.config_alarm_time[i].last_triggred_time == -1)
				{
					eeprom.config_alarm_time[i].last_triggred_time = l_epoch;
					eeprom.config_alarm_time[i].alarm_repetition_cnt = 1;
					write_eeprom((char *)&eeprom);
					convertIdToRowColumn_GlowLedMatrix(eeprom.config_alarm_time[i].Box_ID,2);
					//Blow_Buzzer(100,10000);
					Start_play_sound();
					break;
				}
				else if((l_epoch - eeprom.config_alarm_time[i].last_triggred_time) >= eeprom.config_alarm_time[i].alarm_gap_epoch)    // Need to understand this
				{
					eeprom.config_alarm_time[i].last_triggred_time = l_epoch;
					eeprom.config_alarm_time[i].alarm_repetition_cnt++;					
					write_eeprom((char *)&eeprom);
					convertIdToRowColumn_GlowLedMatrix(eeprom.config_alarm_time[i].Box_ID,2);
					//Blow_Buzzer(100,10000);
					Start_play_sound();
					break;
				}
			}			
		}
		Get_next_alarm_and_Show_on_display((uint8_t)next_compartment,(ret+l_epoch));
	}		
	return 0;
}

void convertIdToRowColumn_GlowLedMatrix(int id,char glow_type) 
{
    // Check if the ID is within the valid range
	static int col, row;
    if (id < 0 || id >= MAXIMUM_SUPPORTED_BOXES) {
        printf("Invalid ID: %d\n", id);
        row = -1;
        col = -1;
        return;
    }

	if(id == 28)
	{
		RGB_Buzzer_Control(0,0,2,0);
		return;
	}

    // Calculate row and column indices
	if(id<4)
	{
		col = MON;
	}
	else if(id<8)
	{
		col = TUE;
	}
	else if(id<12)
	{
		col = WED;
	}
	else if(id<16)
	{
		col = THU;
	}
	else if(id<20)
	{
		col = FRI;
	}
	else if(id<24)
	{
		col = SAT;
	}
	else if(id<28)
	{
		col = SUN;
	}

	if(id%4==0)
	{
		row = MORNING;
	}
	else if(id%4==1)
	{
		row = AFTERNOON;
	}
	else if(id%4==2)
	{
		row = EVENING;
	}
	else if(id%4==3)
	{
		row = NIGHT;
	}
    
	LED28_Control(col,row,glow_type);
}

int32_t Find_next_alarm_and_current_epoch(char *current_alarm, char *next_alarm, uint32_t *current_epoch,uint16_t* next_compartment)
{	
	
	int j=0;
	char flag=0,ret = READ_RTC_and_GET_Epoch(current_epoch);
	int32_t minimum_difference_active=-1,minimum_difference = -1,temp_minimum_difference=0;
	if(eeprom.total_timers_configured>=MAXIMUM_SUPPORTED_TIMER || eeprom.total_timers_configured<=0)
	{
		ESP_LOGW("GGGGGG","-------------------------------");
		return(-1);
	}
	//printf("SSSSSSSSSSSSSSS====%d\n",ret);
	if(ret == 'V')
	{		
		for(int i = 0; i< eeprom.total_timers_configured; i++)
		{
			//printf("T%d=>%u,%u,%u,status=%d\n", i,(unsigned int)eeprom.config_alarm_time[i].timer_epoch,(unsigned int)*current_epoch,(unsigned int)eeprom.config_alarm_time[i].buffer_period_epoch,eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS);
			if(eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS == 1)
			{
				//printf("AT%d=>%u,%u,%u\n", i,(unsigned int)eeprom.config_alarm_time[i].timer_epoch,(unsigned int)*current_epoch,(unsigned int)eeprom.config_alarm_time[i].buffer_period_epoch);
				if(((eeprom.config_alarm_time[i].timer_epoch+eeprom.config_alarm_time[i].total_alarm_duratition_epoch) > (*current_epoch)) &&((eeprom.config_alarm_time[i].timer_epoch-eeprom.config_alarm_time[i].total_alarm_duratition_epoch) <= (*current_epoch)))   
				{
					//printf("BT%d=>%u,%u,%u\n", i,(unsigned int)eeprom.config_alarm_time[i].timer_epoch,(unsigned int)*current_epoch,(unsigned int)eeprom.config_alarm_time[i].buffer_period_epoch);
					current_alarm[i] = 1;
					j++;
				}
				else 
				{
					if(*current_epoch>(eeprom.config_alarm_time[i].timer_epoch+eeprom.config_alarm_time[i].total_alarm_duratition_epoch))
					{
						if(eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS != 3)
						{
							eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS=3;
							write_eeprom((char *)&eeprom);
						}
					}					
				}
			}
		}

		
			for(int i = 0; i < eeprom.total_timers_configured; i++)
			{
				if(eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS == 1)
				{
					if(current_alarm[i] == 1)
					{
						if(minimum_difference_active == -1)
						{
							//*next_alarm = i;
							if( eeprom.config_alarm_time[i].last_triggred_time!=-1)
							{
								minimum_difference_active = (eeprom.config_alarm_time[i].last_triggred_time+eeprom.config_alarm_time[i].alarm_gap_epoch - *current_epoch);
							}
							else
							{
								minimum_difference_active = (*current_epoch-eeprom.config_alarm_time[i].timer_epoch);
							}
						}
						else 
						{						
							if( eeprom.config_alarm_time[i].last_triggred_time!=-1)
							{
								temp_minimum_difference = (eeprom.config_alarm_time[i].last_triggred_time+eeprom.config_alarm_time[i].alarm_gap_epoch - *current_epoch);
							}
							else
							{
								temp_minimum_difference = (*current_epoch-eeprom.config_alarm_time[i].timer_epoch);
							}

							if(temp_minimum_difference<minimum_difference_active)
							{
								minimum_difference_active = temp_minimum_difference;
								//*next_alarm = i;
							}
						}
					}
					else
					{
						if(minimum_difference==-1)
						{
							*next_alarm = i;
							*next_compartment = eeprom.config_alarm_time[i].Box_ID+1;
							minimum_difference = (eeprom.config_alarm_time[i].timer_epoch -*current_epoch);
							
						}
						else if((eeprom.config_alarm_time[i].timer_epoch - *current_epoch) < minimum_difference)
						{
							*next_alarm = i;
							*next_compartment = eeprom.config_alarm_time[i].Box_ID+1;
							minimum_difference = (eeprom.config_alarm_time[i].timer_epoch -*current_epoch);
						}	
					}
									
				}
			}
	}
	else
	{
		ESP_LOGW("KKK","-------------------------------");
		return(-1);
	}
	if(minimum_difference==-1)
	{
		*next_alarm = -1;
		ESP_LOGW("KKK","-212www1212-1----next_alarm -%d----------", *next_alarm);
		return(minimum_difference_active);
	}
	else if(minimum_difference_active==-1)
	{
		return(minimum_difference);
	}
	else if(minimum_difference_active<minimum_difference)
	{
		minimum_difference = minimum_difference_active;
	}
	//ESP_LOGW("KKK","--------------minimum_difference %ld-----------------", minimum_difference);
	return(minimum_difference);
}
 


	