#include "header.h"
/***************************************************/
uint8_t lcd_data[56] = { 0x00 };
ht16c23_config_t ht16c23_conf;
/***************************************************/
void ht16c23_delay_us(int time)
{
	volatile unsigned int i=0;
	for(i=0;i<100;i++);
}

char ht16c23_write(char skip ,char device_add, char reg_add, char data)
{
	char g_ack = 1;
	ht16c23_stop();				//Change
	ht16c23_delay_us(0);
	ht16c23_start();// START COMMUNICATION
	g_ack = ht16c23_tx(device_add);// SELECT DEVICE FOR WRITE PURPOSE

	ht16c23_delay_us(0);
	g_ack = ht16c23_tx(reg_add);// SELECT  REGISTER
	ht16c23_delay_us(0);
	g_ack = ht16c23_tx(data);// SEND DATA
	ht16c23_stop();//STOP COMMUNICATION
	ht16c23_delay_us(0);
	return g_ack;
}

char ht16c23_ram_write(char skip ,char device_add, char reg_add,char seg, char* data)
{
	char g_ack = 1;
	ht16c23_stop();				//Change
	ht16c23_delay_us(0);
	ht16c23_start();// START COMMUNICATION
	g_ack = ht16c23_tx(device_add);// SELECT DEVICE FOR WRITE PURPOSE
	ht16c23_delay_us(0);
	g_ack = ht16c23_tx(reg_add);// SELECT  REGISTER
	ht16c23_delay_us(0);
	g_ack = ht16c23_tx(seg);// SELECT  SEG
	ht16c23_delay_us(0);
	for (int i = 4; i <= 38; i++)
	{
		g_ack = ht16c23_tx(data[i]);// SEND DATA
	}
	
	
	ht16c23_stop();//STOP COMMUNICATION
	ht16c23_delay_us(0);
	return g_ack;
}

char ht16c23_read(char skip,char device_add,char reg_add)
{
	//char ack;//jjjjjjjjjjjjj
	char device_add_read,rdata=0;

	device_add_read = device_add | 0x01;

	ht16c23_start();  // START COMMUNICATION

	 ht16c23_tx(device_add);// SELECT DEVICE FOR WRITE PURPOSE//jjjjjjjjjjjj(ack=ht16c23_tx)
	 ht16c23_tx(reg_add);// SELECT  REGISTER//jjjjjjjjjjjj(ack=ht16c23_tx)
	ht16c23_start();// RESTART COMMUNICATION
	 ht16c23_tx(device_add_read);// SELECT DEVICE FOR READ PURPOSE//jjjjjjjjjjjj(ack=ht16c23_tx)
	rdata = ht16c23_rx();
	/*print_trace("$$$$$$$$$$$$$$$$$$$$ACC DATA===%x",rdata);*/
	ht16c23_stop();
	return(rdata);
}
	
char ht16c23_tx(char x)
{
	char i,ack;
	unsigned char mask = 0x80;
	LCD_SCL_OUTPUT;
	LCD_SDA_OUTPUT;
	ht16c23_delay_us(0);

	for(i=0;i<8;i++)
	{
		ht16c23_delay_us(0);

		if(((x) & (mask)))
		{
			LCD_SDASET;
		}
		else
		{
			LCD_SDACLR;
		}
		ht16c23_delay_us(0);
		LCD_SCLSET;
		ht16c23_delay_us(0);
		LCD_SCLCLR;
		mask >>=1;
	}

	LCD_SDASET;
	ht16c23_delay_us(1);
	ht16c23_delay_us(0);
	LCD_SDA_INPUT;
	esp_rom_delay_us(5);
	ht16c23_delay_us(0);
	LCD_SCLSET;
	ht16c23_delay_us(0);
	if(gpio_get_level(LCD_SDA))
	{
		ack = 1;
	}
	else
	{
		ack = 0;
	}
    // printf("tx ack %d\n", ack);
	LCD_SCLCLR;
	LCD_SDACLR;
	return(ack);
}

// ht16c23_start function definition
void ht16c23_start(void)
{
	LCD_SCL_OUTPUT;
	LCD_SDA_OUTPUT;
	ht16c23_delay_us(0);
	LCD_SDASET;
	ht16c23_delay_us(0);
	LCD_SCLSET;
	ht16c23_delay_us(0);
	LCD_SDACLR;
	ht16c23_delay_us(0);				//Change
	LCD_SCLCLR;						//Change
}

// ht16c23_stop function definition
void ht16c23_stop(void)
{
	LCD_SCL_OUTPUT;
	LCD_SDA_OUTPUT;
	ht16c23_delay_us(0);
	ht16c23_delay_us(0);
	LCD_SDACLR;
	ht16c23_delay_us(0);
	ht16c23_delay_us(0);
	LCD_SCLSET;
	ht16c23_delay_us(0);
	ht16c23_delay_us(0);
	LCD_SDASET;
}

//receive function definition
char ht16c23_rx()
{
	unsigned char d=0,i;
	LCD_SCL_OUTPUT;
	LCD_SDA_INPUT;
	vTaskDelay(100/portTICK_PERIOD_MS);
	ht16c23_delay_us(0);
	ht16c23_delay_us(0);
	for(i=0;i<8;i++)
	{
		LCD_SCLCLR;
		ht16c23_delay_us(0);
		ht16c23_delay_us(0);
		ht16c23_delay_us(0);
		LCD_SCLSET;
		ht16c23_delay_us(0);
		ht16c23_delay_us(0);
		ht16c23_delay_us(0);
		d = d<<1;
		if(gpio_get_level(LCD_SDA))		//(read_char<<i) & 0x80
		{
			d = d | (0x01);
		}
		ht16c23_delay_us(0);
		ht16c23_delay_us(0);
		ht16c23_delay_us(0);
		ht16c23_delay_us(0);
	}
	LCD_SCLCLR;
	ht16c23_delay_us(0);
	ht16c23_delay_us(0);

	//if(d != 0)
	LCD_SDACLR;
	ht16c23_delay_us(0);
	ht16c23_delay_us(0);
	//else
	//	LCD_SDASET;
	LCD_SCLSET;
	ht16c23_delay_us(0);
	LCD_SCLCLR;
	LCD_SDACLR;
	ht16c23_delay_us(0);
	return(d);
}

void ht16c23_init(void)
{
	volatile char who_am_i_reg_read=0x00;
	esp_rom_gpio_pad_select_gpio(GPIO_NUM_21);
    esp_rom_gpio_pad_select_gpio(GPIO_NUM_40);
	printf("HOLTEK IC HT16C23\n");
	vTaskDelay(10/portTICK_PERIOD_MS);
    ht16c23_conf.duty_bias = ht16c23_4DUTY_4BIAS;
    ht16c23_conf.oscillator_display = ht16c23_OSCILLATOR_ON_DISPLAY_ON;
    ht16c23_conf.frame_frequency = ht16c23_FRAME_80HZ;
    ht16c23_conf.blinking_frequency = ht16c23_BLINKING_OFF;
    ht16c23_conf.pin_and_voltage = ht16c23_VLCD_PIN_VOL_ADJ_OFF;
    ht16c23_conf.adjustment_voltage = 15;

    ht16c23_write(0,ht16c23_I2C_WRITE_ADDRESS,ht16c23_CMD_DRIMO,ht16c23_conf.duty_bias);
    ht16c23_write(0,ht16c23_I2C_WRITE_ADDRESS,ht16c23_CMD_SYSMO,ht16c23_conf.oscillator_display);
    ht16c23_write(0,ht16c23_I2C_WRITE_ADDRESS,ht16c23_CMD_FRAME,ht16c23_conf.frame_frequency);
    ht16c23_write(0,ht16c23_I2C_WRITE_ADDRESS,ht16c23_CMD_BLINK,ht16c23_conf.blinking_frequency);
    ht16c23_write(0,ht16c23_I2C_WRITE_ADDRESS,ht16c23_CMD_IVA,ht16c23_conf.pin_and_voltage|ht16c23_conf.adjustment_voltage);
    // memset(lcd_data, 0xFF, sizeof(lcd_data));
    // set_segment(COLUMN_15,7,1);
	// write_Temperature("56.9");
	// show_humidity("95");
	// show_rtc_time("MON","21","55");
	// show_date("120523");
	// show_alarm_time("09","59");
	// show_battery("0");
	// show_Ble_status("1");
	// show_curve("1","1");
	// ht16c23_ram_write(0,ht16c23_I2C_WRITE_ADDRESS,ht16c23_CMD_IOOUT,0x00,(char*)&lcd_data);
}

void set_segment(int column,char row,char value)
{
	f.g_lcd_updated_F = 1;
	if(value == 1)
	{
		lcd_data[column] |=  (0x01<<row);
	}
	else
	{
		lcd_data[column] &=  ~(0x01<<row);
	}
}

void show_rtc_time(char* day,char* hh,char* mm, char mode)
{
   if(strlen(day) == 3)
	{
		write_FND(FND18,day[0]);
		write_FND(FND19,day[1]);
		write_FND(FND20,day[2]);
	}

	if(strlen(hh) == 2)
	{
		write_FND(FND6,hh[0]);
   		write_FND(FND7,hh[1]);
	}
   
	if(strlen(mm) == 2)
	{
		write_FND(FND8,mm[0]);
  		write_FND(FND9,mm[1]);
	}
   set_segment(COLUMN_18,7,1);
}

void show_alarm_time(char* hh,char* mm, char mode)
{  
	if(strlen(hh) == 2)
	{
		write_FND(FND21,hh[0]);
   		write_FND(FND22,hh[1]);
	}
   
	if(strlen(mm) == 2)
	{
		write_FND(FND23,mm[0]);
  		write_FND(FND24,mm[1]);
	}
	set_segment(COLUMN_16,4,1);				// S23
	set_segment(COLUMN_10,4,1);				// S24
   	set_segment(COLUMN_12,4,1);				// S25
}

void show_humidity(char* humidity)
{
	if(strlen(humidity)==2)
	{
		write_FND(FND4,humidity[0]);
		write_FND(FND5,humidity[1]);
		set_segment(COLUMN_33,0,1);
	}
}

void show_battery(char* value)
{
	ESP_LOGI("battery", "<<<%c>>>",value[0]);
	if(strlen(value)==1)
	{
		if(value[0] == '3')
		{
			set_segment(COLUMN_22,2,1);set_segment(COLUMN_22,3,1);set_segment(COLUMN_22,1,1);set_segment(COLUMN_22,0,1);
		}
		else if(value[0] == '2')
		{
			set_segment(COLUMN_22,2,0);set_segment(COLUMN_22,3,1);set_segment(COLUMN_22,1,1);set_segment(COLUMN_22,0,1);
		}
		else if(value[0] == '1')
		{
			set_segment(COLUMN_22,2,0);set_segment(COLUMN_22,3,0);set_segment(COLUMN_22,1,1);set_segment(COLUMN_22,0,1);
		}
		else
		{
			set_segment(COLUMN_22,2,0);set_segment(COLUMN_22,3,0);set_segment(COLUMN_22,1,0);set_segment(COLUMN_22,0,1); 
		}
		
	}
}

void show_Ble_status(char* value)
{
	static char prev_status = 0;
	if(prev_status != value)
	{
		if(strlen(value)==1)
		{
			if(value[0] == '0')
			{
				set_segment(COLUMN_17,3,0);
			}
			else if(value[0] == '1')
			{
				set_segment(COLUMN_17,3,1);
			}			
		}
		prev_status = value[0];
	}	
}

void show_curve(char* left,char* right)
{
	if(left[0]=='1')
	{
		set_segment(COLUMN_20,0,1);
	}
	else
	{
		set_segment(COLUMN_20,0,0);
	}
	if(right[0] == '1')
	{
		set_segment(COLUMN_22,7,1);
	}
	else
	{
		set_segment(COLUMN_22,7,0);
	}
	
	
}

void write_Temperature(char* value)
{
	if(strlen(value)==4)
	{		
		write_FND(FND1,value[0]);
		write_FND(FND2,value[1]);
		write_FND(FND3,value[3]);

		set_segment(COLUMN_12,0,1);				// Dot				//  need to add condition two only one f or c
		if(eeprom.pre_cfg.temperature_unit == 1)
		{
			set_segment(COLUMN_14,0,1);			// f
			set_segment(COLUMN_16,0,0);			// c
		}
		else
		{
			set_segment(COLUMN_14,0,0);			// f
			set_segment(COLUMN_16,0,1);			// c
		}
	}	
}

void write_BoxId(char* value)
{
	if(strlen(value)==2)
	{		
		write_FND(FND25,value[0]);
		write_FND(FND26,value[1]);
	}	
}

void show_date(char* date)
{
	ESP_LOGI("DATE","%s",date);
	if(strlen(date)==6)
	{
		write_FND(FND10,date[0]);
		write_FND(FND11,date[1]);
		write_FND(FND12,date[2]);
		write_FND(FND13,date[3]);
		write_FND(FND14,date[4]);
		write_FND(FND15,date[5]);
		set_segment(COLUMN_23,3,1);			// S18
		set_segment(COLUMN_26,4,1);			// S19
		set_segment(COLUMN_28,4,1);			// S20
	}
}

void write_FND(int FND_ID,char value)
{
	// row start from bottom to top
	f.g_lcd_updated_F = 1;
	switch(value)
	{
		case '0':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,0);
		}break;
		case '1':
		{
			Set_A(FND_ID,0);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,0);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,0);
		}break;
		case '2':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,0);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,0);Set_G(FND_ID,1);
		}break;
		case '3':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,1);
		}break;
		case '4':
		{
			Set_A(FND_ID,0);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,0);Set_E(FND_ID,0);Set_F(FND_ID,1);Set_G(FND_ID,1);
		}break;
		case '5':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,0);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,0);Set_F(FND_ID,1);Set_G(FND_ID,1);
		}break;
		case '6':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,0);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,1);
		}break;
		case '7':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,0);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,0);
		}break;
		case '8':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,1);
		}break;
		case '9':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,0);Set_F(FND_ID,1);Set_G(FND_ID,1);
		}break;
		case 'A':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,0);Set_O(FND_ID,1);Set_P(FND_ID,1);Set_Q(FND_ID,0);
		}break;
		case 'D':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,0);Set_H(FND_ID,0);Set_I(FND_ID,0);Set_J(FND_ID,1);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,1);Set_N(FND_ID,0);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
		case 'E':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,0);Set_D(FND_ID,0);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,0);Set_O(FND_ID,1);Set_P(FND_ID,1);Set_Q(FND_ID,0);
		}break;
		case 'F':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,0);Set_D(FND_ID,0);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,0);Set_O(FND_ID,1);Set_P(FND_ID,1);Set_Q(FND_ID,0);
		}break;
		case 'H':
		{
			Set_A(FND_ID,0);Set_B(FND_ID,0);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,0);Set_O(FND_ID,1);Set_P(FND_ID,1);Set_Q(FND_ID,0);
		}break;
		case 'I':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,0);Set_D(FND_ID,0);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,0);Set_H(FND_ID,0);Set_I(FND_ID,0);Set_J(FND_ID,1);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,1);Set_N(FND_ID,0);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
		case 'M':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,1);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,1);Set_N(FND_ID,0);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
		case 'N':
		{
			Set_A(FND_ID,0);Set_B(FND_ID,0);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,1);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,1);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
		case 'O':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,0);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
		case 'R':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,1);Set_D(FND_ID,0);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,1);Set_O(FND_ID,1);Set_P(FND_ID,1);Set_Q(FND_ID,0);
		}break;
		case 'S':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,0);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,0);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,0);Set_O(FND_ID,1);Set_P(FND_ID,1);Set_Q(FND_ID,0);
		}break;
		case 'T':
		{
			Set_A(FND_ID,1);Set_B(FND_ID,1);Set_C(FND_ID,0);Set_D(FND_ID,0);Set_E(FND_ID,0);Set_F(FND_ID,0);Set_G(FND_ID,0);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,1);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,1);Set_N(FND_ID,0);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
		case 'U':
		{
			Set_A(FND_ID,0);Set_B(FND_ID,0);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,0);Set_N(FND_ID,0);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
		case 'W':
		{
			Set_A(FND_ID,0);Set_B(FND_ID,0);Set_C(FND_ID,1);Set_D(FND_ID,1);Set_E(FND_ID,1);Set_F(FND_ID,1);Set_G(FND_ID,1);Set_H(FND_ID,1);Set_I(FND_ID,0);Set_J(FND_ID,0);Set_K(FND_ID,0);Set_L(FND_ID,0);Set_M(FND_ID,1);Set_N(FND_ID,0);Set_O(FND_ID,0);Set_P(FND_ID,0);Set_Q(FND_ID,0);
		}break;
	}
}


void Set_A(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND1:
		{
			column = COLUMN_09;
			row = 0;
		}break;

		case FND2:
		{
			column = COLUMN_11;
			row = 0;
		}break;
		
		case FND3:
		{
			column = COLUMN_13;
			row = 0;
		}break;
		case FND4:
		{
			column = COLUMN_15;
			row = 0;
		}break;
		case FND5:
		{
			column = COLUMN_34;
			row = 0;
		}break;
		case FND6:
		{
			column = COLUMN_18;
			row = 3;
		}break;
		case FND7:
		{
			column = COLUMN_17;
			row = 7;
		}break;
		case FND8:
		{
			column = COLUMN_19;
			row = 7;
		}break;
		
		case FND9:
		{
			column = COLUMN_21;
			row = 7;
		}break;

		case FND10:
		{
			column = COLUMN_19;
			row = 3;
		}break;

		case FND11:
		{
			column = COLUMN_21;
			row = 3;
		}break;

		case FND12:
		{
			column = COLUMN_23;
			row = 7;
		}break;

		case FND13:
		{
			column = COLUMN_25;
			row = 7;
		}break;

		case FND14:
		{
			column = COLUMN_27;
			row = 7;
		}break;
		case FND15:
		{
			column = COLUMN_29;
			row = 7;
		}break;
		case FND16:
		{
			return;
			// column = COLUMN_38;
			// row = 0;
		}break;
		case FND17:
		{
			return;
			// column = COLUMN_24;
			// row = 3;
		}break;
		case FND18:
		{
			column = COLUMN_24;
			row = 3;
		}break;
		case FND19:
		{
			column = COLUMN_28;
			row = 3;
		}break;
		case FND20:
		{
			column = COLUMN_30;
			row = 4;
		}break;
		case FND21:
		{
			column = COLUMN_09;
			row = 4;
		}break;
		case FND22:
		{
			column = COLUMN_11;
			row = 4;
		}break;
		case FND23:
		{
			column = COLUMN_13;
			row = 4;
		}break;
		case FND24:
		{
			column = COLUMN_15;
			row = 4;
		}break;
		case FND25:
		{
			column = COLUMN_34;
			row = 4;
		}break;
		case FND26:
		{
			column = COLUMN_32;
			row = 4;
		}break;
		// case FND27:
		// {
		// 	column = COLUMN_34;
		// 	row = 2;
		// }break;
	}
	set_segment(column,row,value);
}

void Set_B(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND1:
		{
			column = COLUMN_10;
			row = 1;
		}break;

		case FND2:
		{
			column = COLUMN_12;
			row = 1;
		}break;
		
		case FND3:
		{
			column = COLUMN_14;
			row = 1;
		}break;
		case FND4:
		{
			column = COLUMN_16;
			row = 1;
		}break;
		case FND5:
		{
			column = COLUMN_33;
			row = 1;
		}break;
		case FND6:
		{
			column = COLUMN_17;
			row = 2;
		}break;
		case FND7:
		{
			column = COLUMN_18;
			row = 6;
		}break;
		case FND8:
		{
			column = COLUMN_20;
			row = 7;
		}break;
		case FND9:
		{
			column = COLUMN_22;
			row = 6;
		}break;
		case FND10:
		{
			column = COLUMN_20;
			row = 3;
		}break;

		case FND11:
		{
			column = COLUMN_23;
			row = 2;
		}break;

		case FND12:
		{
			column = COLUMN_24;
			row = 7;
		}break;
		case FND13:
		{
			column = COLUMN_26;
			row = 7;
		}break;
		case FND14:
		{
			column = COLUMN_28;
			row = 7;
		}break;
		case FND15:
		{
			column = COLUMN_30;
			row = 7;
		}break;
		case FND16:
		{
			
			// column = COLUMN_37;
			// row = 1;
		}break;
		case FND17:
		{
			// column = COLUMN_35;
			// row = 0;
		}break;
		case FND18:
		{
			column = COLUMN_26;
			row = 3;
		}break;
		case FND19:
		{
			column = COLUMN_29;
			row = 3;
		}break;
		case FND20:
		{
			column = COLUMN_32;
			row = 3;
		}break;
		case FND21:
		{
			column = COLUMN_10;
			row = 5;
		}break;
		case FND22:
		{
			column = COLUMN_12;
			row = 5;
		}break;
		case FND23:
		{
			column = COLUMN_14;
			row = 5;
		}break;
		case FND24:
		{
			column = COLUMN_16;
			row = 5;
		}break;
		case FND25:
		{
			column = COLUMN_33;
			row = 5;
		}break;
		case FND26:
		{
			column = COLUMN_31;
			row = 5;
		}break;
		// case FND27:
		// {
		// 	column = COLUMN_34;
		// 	row = 2;
		// }break;
	}
	set_segment(column,row,value);
}


void Set_C(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND1:
		{
			column = COLUMN_10;
			row = 3;
		}break;

		case FND2:
		{
			column = COLUMN_12;
			row = 3;
		}break;
		
		case FND3:
		{
			column = COLUMN_14;
			row = 3;
		}break;

		case FND4:
		{
			column = COLUMN_16;
			row = 3;
		}break;

		case FND5:
		{
			column = COLUMN_33;
			row = 3;
		}break;
		case FND6:
		{
			column = COLUMN_17;
			row = 0;
		}break;
		case FND7:
		{
			column = COLUMN_18;
			row = 4;
		}break;
		case FND8:
		{
			column = COLUMN_20;
			row = 5;
		}break;

		case FND9:
		{
			column = COLUMN_22;
			row = 4;
		}break;

		case FND10:
		{
			column = COLUMN_20;
			row = 1;
		}break;
		case FND11:
		{
			column = COLUMN_23;
			row = 0;
		}break;

		case FND12:
		{
			column = COLUMN_24;
			row = 5;
		}break;

		case FND13:
		{
			column = COLUMN_26;
			row = 5;
		}break;
		case FND14:
		{
			column = COLUMN_28;
			row = 5;
		}break;
		case FND15:
		{
			column = COLUMN_30;
			row = 5;
		}break;
		case FND16:
		{
			// column = COLUMN_37;
			// row = 3;
		}break;
		case FND17:
		{
			// column = COLUMN_35;
			// row = 2;
		}break;
		case FND18:
		{
			column = COLUMN_26;
			row = 2;
		}break;
		case FND19:
		{
			column = COLUMN_29;
			row = 2;
		}break;
		case FND20:
		{
			column = COLUMN_32;
			row = 2;
		}break;
		case FND21:
		{
			column = COLUMN_10;
			row = 7;
		}break;
		case FND22:
		{
			column = COLUMN_12;
			row = 7;
		}break;
		case FND23:
		{
			column = COLUMN_14;
			row = 7;
		}break;
		case FND24:
		{
			column = COLUMN_16;
			row = 7;
		}break;
		case FND25:
		{
			column = COLUMN_33;
			row = 7;
		}break;
		case FND26:
		{
			column = COLUMN_31;
			row = 7;
		}break;
		// case FND27:
		// {
		// 	column = COLUMN_34;
		// 	row = 3;
		// }break;
	}
	set_segment(column,row,value);
}

void Set_D(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND1:
		{
			column = COLUMN_09;
			row = 3;
		}break;

		case FND2:
		{
			column = COLUMN_11;
			row = 3;
		}break;
		
		case FND3:
		{
			column = COLUMN_13;
			row = 3;
		}break;
		case FND4:
		{
			column = COLUMN_15;
			row = 3;
		}break;
		case FND5:
		{
			column = COLUMN_34;
			row = 3;
		}break;
		case FND6:
		{
			column = COLUMN_18;
			row = 0;
		}break;
		case FND7:
		{
			column = COLUMN_17;
			row = 4;
		}break;
		case FND8:
		{
			column = COLUMN_19;
			row = 4;
		}break;
		case FND9:
		{
			column = COLUMN_21;
			row = 4;
		}break;

		case FND10:
		{
			column = COLUMN_19;
			row = 0;
		}break;

		case FND11:
		{
			column = COLUMN_21;
			row = 0;
		}break;
		case FND12:
		{
			column = COLUMN_23;
			row = 4;
		}break;


		case FND13:
		{
			column = COLUMN_25;
			row = 4;
		}break;
		case FND14:
		{
			column = COLUMN_27;
			row = 4;
		}break;
		case FND15:
		{
			column = COLUMN_29;
			row = 4;
		}break;
		case FND16:
		{
			// column = COLUMN_38;
			// row = 3;
		}break;
		case FND17:
		{
			// column = COLUMN_36;
			// row = 3;
		}break;
		case FND18:
		{
			column = COLUMN_26;
			row = 1;
		}break;
		case FND19:
		{
			column = COLUMN_29;
			row = 1;
		}break;
		case FND20:
		{
			column = COLUMN_32;
			row = 0;
		}break;
		case FND21:
		{
			column = COLUMN_09;
			row = 7;
		}break;
		case FND22:
		{
			column = COLUMN_11;
			row = 7;
		}break;
		case FND23:
		{
			column = COLUMN_13;
			row = 7;
		}break;
		case FND24:
		{
			column = COLUMN_15;
			row = 7;
		}break;
		case FND25:
		{
			column = COLUMN_34;
			row = 7;
		}break;
		case FND26:
		{
			column = COLUMN_32;
			row = 7;
		}break;
		// case FND27:
		// {
		// 	column = COLUMN_34;
		// 	row = 2;
		// }break;
	}
	set_segment(column,row,value);
}

void Set_E(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND1:
		{
			column = COLUMN_09;
			row = 2;
		}break;

		case FND2:
		{
			column = COLUMN_11;
			row = 2;
		}break;
		
		case FND3:
		{
			column = COLUMN_13;
			row = 2;
		}break;

		case FND4:
		{
			column = COLUMN_15;
			row = 2;
		}break;
		case FND5:
		{
			column = COLUMN_34;
			row = 2;
		}break;
		case FND6:
		{
			column = COLUMN_18;
			row = 1;
		}break;
		case FND7:
		{
			column = COLUMN_17;
			row = 5;
		}break;
		case FND8:
		{
			column = COLUMN_19;
			row = 5;
		}break;
		case FND9:
		{
			column = COLUMN_21;
			row = 5;
		}break;
		case FND10:
		{
			column = COLUMN_19;
			row = 1;
		}break;

		case FND11:
		{
			column = COLUMN_21;
			row = 1;
		}break;

		case FND12:
		{
			column = COLUMN_23;
			row = 5;
		}break;

		case FND13:
		{
			column = COLUMN_25;
			row = 5;
		}break;
		case FND14:
		{
			column = COLUMN_27;
			row = 5;
		}break;
		case FND15:
		{
			column = COLUMN_29;
			row = 5;
		}break;
		case FND16:
		{
			// column = COLUMN_38;
			// row = 2;
		}break;
		case FND17:
		{
			// column = COLUMN_36;
			// row = 2;
		}break;
		case FND18:
		{
			column = COLUMN_26;
			row = 0;
		}break;
		case FND19:
		{
			column = COLUMN_28;
			row = 0;
		}break;
		case FND20:
		{
			column = COLUMN_31;
			row = 0;
		}break;
		case FND21:
		{
			column = COLUMN_09;
			row = 6;
		}break;
		case FND22:
		{
			column = COLUMN_11;
			row = 6;
		}break;
		case FND23:
		{
			column = COLUMN_13;
			row = 6;
		}break;
		case FND24:
		{
			column = COLUMN_15;
			row = 6;
		}break;
		case FND25:
		{
			column = COLUMN_34;
			row = 6;
		}break;
		case FND26:
		{
			column = COLUMN_32;
			row = 6;
		}break;
		// case FND27:
		// {
		// 	column = COLUMN_34;
		// 	row = 1;
		// }break;
	}
	set_segment(column,row,value);
}

void Set_F(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND1:
		{
			column = COLUMN_09;
			row = 1;
		}break;

		case FND2:
		{
			column = COLUMN_11;
			row = 1;
		}break;
		
		case FND3:
		{
			column = COLUMN_13;
			row = 1;
		}break;
		case FND4:
		{
			column = COLUMN_15;
			row = 1;
		}break;
		case FND5:
		{
			column = COLUMN_34;
			row = 1;
		}break;
		case FND6:
		{
			column = COLUMN_18;
			row = 2;
		}break;
		case FND7:
		{
			column = COLUMN_17;
			row = 6;
		}break;
		case FND8:
		{
			column = COLUMN_19;
			row = 6;
		}break;
		case FND9:
		{
			column = COLUMN_21;
			row = 6;
		}break;
		case FND10:
		{
			column = COLUMN_19;
			row = 2;
		}break;

		case FND11:
		{
			column = COLUMN_21;
			row = 2;
		}break;

		case FND12:
		{
			column = COLUMN_23;
			row = 6;
		}break;

		case FND13:
		{
			column = COLUMN_25;
			row = 6;
		}break;
		case FND14:
		{
			column = COLUMN_27;
			row = 6;
		}break;
		case FND15:
		{
			column = COLUMN_29;
			row = 6;
		}break;
		case FND16:
		{
			// column = COLUMN_38;
			// row = 1;
		}break;
		case FND17:
		{
			// column = COLUMN_36;
			// row =1;
		}break;
		case FND18:
		{
			column = COLUMN_25;
			row = 0;
		}break;
		case FND19:
		{
			column = COLUMN_27;
			row = 0;
		}break;
		case FND20:
		{
			column = COLUMN_30;
			row = 0;
		}break;
		case FND21:
		{
			column = COLUMN_09;
			row = 5;
		}break;
		case FND22:
		{
			column = COLUMN_11;
			row = 5;
		}break;
		case FND23:
		{
			column = COLUMN_13;
			row = 5;
		}break;
		case FND24:
		{
			column = COLUMN_15;
			row = 5;
		}break;
		case FND25:
		{
			column = COLUMN_34;
			row = 5;
		}break;
		case FND26:
		{
			column = COLUMN_32;
			row = 5;
		}break;
		// case FND27:
		// {
		// 	return;
		// }break;
	}
	set_segment(column,row,value);
}

void Set_G(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND1:
		{
			column = COLUMN_10;
			row = 2;
		}break;

		case FND2:
		{
			column = COLUMN_12;
			row = 2;
		}break;
		
		case FND3:
		{
			column = COLUMN_14;
			row = 2;
		}break;
		
		case FND4:
		{
			column = COLUMN_16;
			row = 2;
		}break;
		case FND5:
		{
			column = COLUMN_33;
			row = 2;
		}break;
		case FND6:
		{
			column = COLUMN_17;
			row = 1;
		}break;
		case FND7:
		{
			column = COLUMN_18;
			row = 5;
		}break;
		case FND8:
		{
			column = COLUMN_20;
			row = 6;
		}break;
		case FND9:
		{
			column = COLUMN_22;
			row = 5;
		}break;

		case FND10:
		{
			column = COLUMN_20;
			row = 2;
		}break;

		case FND11:
		{
			column = COLUMN_23;
			row = 1;
		}break;

		case FND12:
		{
			column = COLUMN_24;
			row = 6;
		}break;
		case FND13:
		{
			column = COLUMN_26;
			row = 6;
		}break;
		case FND14:
		{
			column = COLUMN_28;
			row = 6;
		}break;
		case FND15:
		{
			column = COLUMN_30;
			row = 6;
		}break;
		case FND16:
		{
			// column = COLUMN_37;
			// row = 2;
		}break;
		case FND17:
		{
			// column = COLUMN_35;
			// row = 1;
		}break;
		case FND18:
		{
			column = COLUMN_24;
			row = 0;
		}break;
		case FND19:
		{
			column = COLUMN_27;
			row = 1;
		}break;
		case FND20:
		{
			column = COLUMN_29;
			row = 0;
		}break;
		case FND21:
		{
			column = COLUMN_10;
			row = 6;
		}break;
		case FND22:
		{
			column = COLUMN_12;
			row = 6;
		}break;
		case FND23:
		{
			column = COLUMN_14;
			row = 6;
		}break;
		case FND24:
		{
			column = COLUMN_16;
			row = 6;
		}break;
		case FND25:
		{
			column = COLUMN_33;
			row = 6;
		}break;
		case FND26:
		{
			column = COLUMN_31;
			row = 6;
		}break;
		// case FND27:
		// {
		// 	column = COLUMN_34;
		// 	row = 2;
		// }break;
	}
	set_segment(column,row,value);
}

void Set_H(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			column = COLUMN_24;
			row = 2;
		}break;
		case FND19:
		{
			column = COLUMN_27;
			row = 3;
		}break;
		case FND20:
		{
			column = COLUMN_30;
			row = 3;
		}break;
	}
	set_segment(column,row,value);
}

void Set_I(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			return;
		}break;
		case FND19:
		{
			return;
		}break;
		case FND20:
		{
			column = COLUMN_30;
			row = 2;
		}break;
	}
	set_segment(column,row,value);
}

void Set_J(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			column = COLUMN_25;
			row = 3;
		}break;
		case FND19:
		{
			return;
		}break;
		case FND20:
		{
			column = COLUMN_31;
			row = 3;
		}break;
	}
	set_segment(column,row,value);
}

void Set_K(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			return;
		}break;
		case FND19:
		{
			return;
		}break;
		case FND20:
		{
			return;
		}break;
	}
	set_segment(column,row,value);
}

void Set_L(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			return;
		}break;
		case FND19:
		{
			return;
		}break;
		case FND20:
		{
			return;
		}break;
	}
	set_segment(column,row,value);
}

void Set_M(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			column = COLUMN_25;
			row = 1;
		}break;
		case FND19:
		{
			return;
		}break;
		case FND20:
		{
			column = COLUMN_31;
			row = 1;
		}break;
	}
	set_segment(column,row,value);
}

void Set_N(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			return;
		}break;
		case FND19:
		{
			column = COLUMN_28;
			row = 1;
		}break;
		case FND20:
		{
			column = COLUMN_32;
			row = 1;
		}break;
	}
	set_segment(column,row,value);
}

void Set_O(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			column = COLUMN_25;
			row = 2;
		}break;
		case FND19:
		{
			column = COLUMN_28;
			row = 2;
		}break;
		case FND20:
		{
			column = COLUMN_31;
			row = 2;
		}break;
	}
	set_segment(column,row,value);
}

void Set_P(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			column = COLUMN_24;
			row = 1;
		}break;
		case FND19:
		{
			column = COLUMN_27;
			row = 2;
		}break;
		case FND20:
		{
			column = COLUMN_30;
			row = 1;
		}break;
	}
	set_segment(column,row,value);
}

void Set_Q(int FND_ID,char value)
{
	int column = 0;
	char row = 0;
	switch(FND_ID)
	{
		case FND18:
		{
			return;
		}break;
		case FND19:
		{
			return;
		}break;
		case FND20:
		{
			return;
		}break;
	}
	set_segment(column,row,value);
}

void handle_display()
{
	
	if(f.g_lcd_updated_F == 1)
	{
		if (f.g_backlight_Off_F == 1)
        {
            BACKLIGHT_OFF;
        }
		else
		{
			BACKLIGHT_ON;
		}
		f.g_lcd_updated_F=0;

		if(f.g_display_Off_F == 1)
		{
			f.g_display_Off_F = 0;
			memset(lcd_data, 0, sizeof(lcd_data));
		}
		ht16c23_ram_write(0,ht16c23_I2C_WRITE_ADDRESS,ht16c23_CMD_IOOUT,0x00,(char*)&lcd_data);
	}      
}











