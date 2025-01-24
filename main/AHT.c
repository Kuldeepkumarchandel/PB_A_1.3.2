#include "header.h"

// Structure to store temperature and humidity data from AHT20 sensor
_AHT_DATA g_aht_data;

// Initialize I2C GPIO pins for AHT20 sensor communication
void Init_I2C_sensor()
{
	gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

// Generate I2C start condition
void I2C_Start(void)
{
	AHT_SDASET;
	delay_us(8);
	AHT_SCLSET;
	delay_us(8);
	AHT_SDACLR;
	delay_us(8);				//Change
	AHT_SCLCLR;	
	delay_us(8);				//Change
}

// Generate I2C stop condition
void Stop_I2C(void)
{
    AHT_SDACLR;
    delay_us(8);
    AHT_SCLSET;
    delay_us(8);
    AHT_SDASET;
    delay_us(8);
}

// Wait for and receive ACK from slave device
// Returns 1 if ACK received, 0 if timeout
uint8_t Receive_ACK(void)   
{
	uint16_t CNT;
	CNT = 0;
	AHT_SCLCLR;	
    AHT_SDAFLOAT;
	esp_rom_delay_us(8);	
	AHT_SCLSET;	
	esp_rom_delay_us(8);	
	while(gpio_get_level(AHT_SDA) && CNT < 100) 
	CNT++;
	if(CNT == 100)
	{
		return 0;
	}
 	AHT_SCLCLR;	
	esp_rom_delay_us(8);	
	return 1;
}

// Send ACK to slave device
void Send_ACK(void)		  
{
	AHT_SCLCLR;	
	esp_rom_delay_us(8);	
	AHT_SDACLR;
	esp_rom_delay_us(8);	
	AHT_SCLSET;	
	esp_rom_delay_us(8);
	AHT_SCLCLR;	
	esp_rom_delay_us(8);
	AHT_SDAFLOAT;
	esp_rom_delay_us(8);
}

// Send NACK to slave device
void Send_NOT_ACK(void)	
{
	AHT_SCLCLR;	
	esp_rom_delay_us(8);
	AHT_SDASET;
	esp_rom_delay_us(8);
	AHT_SCLSET;	
	esp_rom_delay_us(8);		
	AHT_SCLCLR;	
	esp_rom_delay_us(8);
    AHT_SDACLR;
	esp_rom_delay_us(8);   
}

// Write a byte to AHT20 sensor
void AHT20_WR_Byte(uint8_t Byte)
{
	uint8_t Data,N,i;	
	Data=Byte;
	i = 0x80;
	for(N=0;N<8;N++)
	{
		AHT_SCLCLR; 
		esp_rom_delay_us(4);  	
		if(i&Data)
		{
			AHT_SDASET;
		}
		else
		{
			AHT_SDACLR;
		}	
			
       AHT_SCLSET;
	    esp_rom_delay_us(4);  
		Data <<= 1;
	}
	AHT_SCLCLR;
	esp_rom_delay_us(8);  
	AHT_SDAFLOAT;
	esp_rom_delay_us(8);	
}	

// Read a byte from AHT20 sensor
uint8_t AHT20_RD_Byte(void)
{
	uint8_t Byte,i,a;
	Byte = 0;
	AHT_SCLCLR;
	AHT_SDAFLOAT;
	esp_rom_delay_us(8);	
	for(i=0;i<8;i++)
	{
    AHT_SCLSET;		
		esp_rom_delay_us(10);
		a=0;
		if(gpio_get_level(AHT_SDA)) a=1;
		Byte = (Byte<<1)|a;
		AHT_SCLCLR;
		esp_rom_delay_us(10);
	}
  AHT_SDAFLOAT;
	esp_rom_delay_us(8);
	return Byte;
}

// Read status byte from AHT20 sensor
uint8_t AHT20_Read_Status(void)
{
    uint8_t Byte_first;	
	I2C_Start();
	AHT20_WR_Byte(0x71);
	Receive_ACK();
	Byte_first = AHT20_RD_Byte();
	Send_NOT_ACK();
	Stop_I2C();
	return Byte_first;
}

// Calculate CRC8 checksum for data validation
uint8_t Calc_CRC8(uint8_t *message,uint8_t Num)
{
        uint8_t i;
        uint8_t byte;
        uint8_t crc=0xFF;
  for(byte=0; byte<Num; byte++)
  {
    crc^=(message[byte]);
    for(i=8;i>0;--i)
    {
      if(crc&0x80) crc=(crc<<1)^0x31;
      else crc=(crc<<1);
    }
  }
        return crc;
}

// Send measurement command to AHT20
void AHT20_SendAC(void) 
{
	I2C_Start();
	AHT20_WR_Byte(0x70);
	Receive_ACK();
	AHT20_WR_Byte(0xac);
	Receive_ACK();
	AHT20_WR_Byte(0x33);
	Receive_ACK();
	AHT20_WR_Byte(0x00);
	Receive_ACK();
	Stop_I2C();
}

// Read temperature and humidity data with CRC check
void AHT20_Read_CTdata_crc(uint32_t *ct) 
{
	volatile uint8_t  Byte_1th=0;
	volatile uint8_t  Byte_2th=0;
	volatile uint8_t  Byte_3th=0;
	volatile uint8_t  Byte_4th=0;
	volatile uint8_t  Byte_5th=0;
	volatile uint8_t  Byte_6th=0;
	volatile uint8_t  Byte_7th=0;
	 uint32_t RetuData = 0;
	 uint16_t cnt = 0;
	 uint16_t cnt1 = 0;
	 uint8_t  CTDATA[6]={0};
	
	AHT20_SendAC();
	vTaskDelay(80/portTICK_PERIOD_MS);
    cnt = 0;
	while(((AHT20_Read_Status()&0x80)==0x80))
	{
		esp_rom_delay_us(1508);
		if(cnt++>=100)
		{
			cnt = 5000;
		 	break;
		}
	}
	
	// Read 7 bytes of data including CRC
	I2C_Start();
	AHT20_WR_Byte(0x71);
	Receive_ACK();
	CTDATA[0]=Byte_1th = AHT20_RD_Byte();
	Send_ACK();
	CTDATA[1]=Byte_2th = AHT20_RD_Byte();
	Send_ACK();
	CTDATA[2]=Byte_3th = AHT20_RD_Byte();
	Send_ACK();
	CTDATA[3]=Byte_4th = AHT20_RD_Byte();
	Send_ACK();
	CTDATA[4]=Byte_5th = AHT20_RD_Byte();
	Send_ACK();
	CTDATA[5]=Byte_6th = AHT20_RD_Byte();
	Send_ACK();
	Byte_7th = AHT20_RD_Byte();
	Send_NOT_ACK();                        
	Stop_I2C();
	
	// Verify CRC and process data if valid
	if(Calc_CRC8(CTDATA,6)==Byte_7th)
	{
		cnt1 = 120;
		// Process humidity data
		RetuData = (RetuData|Byte_2th)<<8;
		RetuData = (RetuData|Byte_3th)<<8;
		RetuData = (RetuData|Byte_4th);
		RetuData =RetuData >>4;
		ct[0] = RetuData;
		
		// Process temperature data
		RetuData = 0;
		RetuData = (RetuData|Byte_4th)<<8;
		RetuData = (RetuData|Byte_5th)<<8;
		RetuData = (RetuData|Byte_6th);
		RetuData = RetuData&0xfffff;
		ct[1] =RetuData; 
	}
	else
	{
		ct[0]=0x00;
		ct[1]=0x00;
	}
}

// Reset AHT20 register
void JH_Reset_REG(uint8_t addr)
{
	uint8_t Byte_first,Byte_second,Byte_third,Byte_fourth;
	
	// Read current register values
	I2C_Start();
	AHT20_WR_Byte(0x70);
	Receive_ACK();
	AHT20_WR_Byte(addr);
	Receive_ACK();
	AHT20_WR_Byte(0x00);
	Receive_ACK();
	AHT20_WR_Byte(0x00);
	Receive_ACK();
	Stop_I2C();

	vTaskDelay(5/portTICK_PERIOD_MS);
	
	I2C_Start();
	AHT20_WR_Byte(0x71);
	Receive_ACK();
	Byte_first = AHT20_RD_Byte();
	Send_ACK();
	Byte_second = AHT20_RD_Byte();
	Send_ACK();
	Byte_third = AHT20_RD_Byte();
	Send_NOT_ACK();
	Stop_I2C();
	
    vTaskDelay(10/portTICK_PERIOD_MS);
	
	// Write back modified values
	I2C_Start();
	AHT20_WR_Byte(0x70);
	Receive_ACK();
	AHT20_WR_Byte(0xB0|addr);
	Receive_ACK();
	AHT20_WR_Byte(Byte_second);
	Receive_ACK();
	AHT20_WR_Byte(Byte_third);
	Receive_ACK();
	Stop_I2C();
	
	Byte_second=0x00;
	Byte_third =0x00;
}

// Initialize AHT20 by resetting registers
void AHT20_Start_Init(void)
{
	JH_Reset_REG(0x1b);
	JH_Reset_REG(0x1c);
	JH_Reset_REG(0x1e);
}

// Read temperature and humidity and update display
void Read_and_show_temp_humidity()
{
	uint32_t CT_data[2];
	char Temp[5] = {0};
	
	// Read sensor data
	AHT20_Read_CTdata_crc(CT_data);
	
	// Convert raw values to actual temperature and humidity
	g_aht_data.humidity_data = CT_data[0]*100/1024/1024;
	g_aht_data.temp_data     = CT_data[1]*200/1024/1024-50;

	ESP_LOGI("TEMP_DATA", " Temp:%f c", g_aht_data.temp_data);
	
	// Format and display temperature
	if(eeprom.pre_cfg.temperature_unit == 1)
	{
		g_aht_data.temp_data = g_aht_data.temp_data*9/5+32;    //  Convert to Fahrenheit
		ESP_LOGI("TEMP_DATA", " Temp:%f f", g_aht_data.temp_data);
	}
	else
	{
		g_aht_data.temp_data = g_aht_data.temp_data;  	  //  Celsius
		ESP_LOGI("TEMP_DATA", " Temp:%f c", g_aht_data.temp_data);
	}
	sprintf((char *)Temp, "%03.1f",g_aht_data.temp_data);
	Temp[4]=0;
	write_Temperature((char*)&Temp);
	PB_reading.temperature2 =  atoi(&Temp[3]);
	Temp[2]=0;
	PB_reading.temperature1 = atoi(Temp);

	// Format and display humidity
	sprintf((char *)Temp, "%02d",(int)g_aht_data.humidity_data );
	Temp[2]=0;
	PB_reading.humidity = atoi(Temp);
	show_humidity((char*)&Temp);
	ESP_LOGI("TEMP_DATA", " Humidity:%f, %f, %ld, %ld", (float)PB_reading.humidity,g_aht_data.temp_data,CT_data[0],CT_data[1]);
}

// Initialize AHT20 sensor
void AHT20_INIT()
{
   Init_I2C_sensor();
   
   // Initialize sensor with configuration commands
   I2C_Start();
   AHT20_WR_Byte(0X70);
	Receive_ACK();
	AHT20_WR_Byte(0xa8);
	Receive_ACK();
	AHT20_WR_Byte(0x00);
	Receive_ACK();
	AHT20_WR_Byte(0x00);
	Receive_ACK();
	Stop_I2C();

	vTaskDelay(10/portTICK_PERIOD_MS);

	I2C_Start();
	AHT20_WR_Byte(0x70);
	Receive_ACK();
	AHT20_WR_Byte(0xbe);
	Receive_ACK();
	AHT20_WR_Byte(0x08);
	Receive_ACK();
	AHT20_WR_Byte(0x00);
	Receive_ACK();
	Stop_I2C();
	vTaskDelay(10/portTICK_PERIOD_MS);
}