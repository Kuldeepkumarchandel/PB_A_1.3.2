#ifndef _AHT_H_
#define _AHT_H_

typedef struct
{
    float humidity_data;
    float temp_data;
}_AHT_DATA;

extern _AHT_DATA g_aht_data;

#define AHT_SCL					SCLL
#define AHT_SDA					SDAA
#define GPIO_OUTPUT_PIN_SEL  	((1ULL<<AHT_SCL) | (1ULL<<AHT_SDA))
#define AHT_DEVICE_ADDR 		0x38

#define AHT_SCLSET 				gpio_set_level(AHT_SCL, 1);
#define AHT_SCLCLR 				gpio_set_level(AHT_SCL, 0);

#define AHT_SCL_OUTPUT 			esp_rom_gpio_pad_select_gpio(AHT_SCL); gpio_set_direction(AHT_SCL, GPIO_MODE_OUTPUT);   
#define AHT_SDA_OUTPUT  		esp_rom_gpio_pad_select_gpio(AHT_SDA); gpio_set_direction(AHT_SDA, GPIO_MODE_OUTPUT);
#define AHT_SDA_INPUT			esp_rom_gpio_pad_select_gpio(AHT_SDA); gpio_set_direction(AHT_SDA, GPIO_MODE_INPUT); gpio_pullup_en(AHT_SDA);

#define AHT_SDASET 				AHT_SDA_OUTPUT gpio_set_level(AHT_SDA, 1);
#define AHT_SDACLR 				AHT_SDA_OUTPUT gpio_set_level(AHT_SDA, 0);

#define AHT_SCLRESET			gpio_reset_pin(AHT_SCL);
#define AHT_SDAFLOAT			AHT_SDA_INPUT


void I2C_Start(void)	;
void Stop_I2C(void)	  ;
uint8_t Receive_ACK(void)  ;
void Send_ACK(void)	;
void Send_NOT_ACK(void)	;
void AHT20_WR_Byte(uint8_t Byte);
uint8_t AHT20_RD_Byte(void);
uint8_t AHT20_Read_Status(void);
uint8_t Calc_CRC8(uint8_t *message,uint8_t Num);
void AHT20_SendAC(void);
void AHT20_Read_CTdata_crc(uint32_t *ct) ;
void AHT20_INIT();
void AHT20_Start_Init(void);
void JH_Reset_REG(uint8_t addr);
void Init_I2C_sensor();
void Read_and_show_temp_humidity();
#endif