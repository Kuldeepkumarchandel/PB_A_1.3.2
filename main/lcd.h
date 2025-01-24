#ifndef LCD_H_
#define LCD_H_


extern uint8_t lcd_data[56];

#define MON                     7
#define TUE                     6
#define WED                     5
#define THU                     4
#define FRI                     3
#define SAT                     2
#define SUN                     1


#define RGB_Green_LED           1
#define RGB_Red_LED             2
#define RGB_Blue_LED            3
#define MORNING                 4
#define AFTERNOON               5
#define EVENING                 6
#define NIGHT                   7



// #define COLUMN_38               4
// #define COLUMN_37               1
// #define COLUMN_36               2
// #define COLUMN_35               3
#define COLUMN_34               4
#define COLUMN_33               5
#define COLUMN_32               6
#define COLUMN_31               7
#define COLUMN_30               8
#define COLUMN_29               9
#define COLUMN_28               10
#define COLUMN_27               11
#define COLUMN_26               12
#define COLUMN_25               13
#define COLUMN_24               14
#define COLUMN_23               15
#define COLUMN_22               16
#define COLUMN_21               17
#define COLUMN_20               18
#define COLUMN_19               19
#define COLUMN_18               20
#define COLUMN_17               21
#define COLUMN_16               31
#define COLUMN_15               32
#define COLUMN_14               33
#define COLUMN_13               34
#define COLUMN_12               35
#define COLUMN_11               36
#define COLUMN_10               37
#define COLUMN_09               38

#define FND1                    1
#define FND2                    2
#define FND3                    3
#define FND4                    4
#define FND5                    5
#define FND6                    6
#define FND7                    7
#define FND8                    8
#define FND9                    9
#define FND10                   10
#define FND11                   11
#define FND12                   12
#define FND13                   13
#define FND14                   14
#define FND15                   15
#define FND16                   16
#define FND17                   17
#define FND18                   18
#define FND19                   19
#define FND20                   20
#define FND21                   21
#define FND22                   22
#define FND23                   23
#define FND24                   24
#define FND25                   25
#define FND26                   26
// #define FND27                   27


#define  TIME_MODE_24HR         0
#define  TIME_MODE_AM           1
#define  TIME_MODE_PM           2

#define ht16c23_I2C_WRITE_ADDRESS   (0x7C)
#define ESP_INTR_FLG_DEFAULT  (0)
#define ESP_I2C_MASTER_BUF_LEN  (0)


#define LCD_SCL_OUTPUT      gpio_set_direction(LCD_SCL, GPIO_MODE_OUTPUT)
#define LCD_SDA_OUTPUT      gpio_set_direction(LCD_SDA, GPIO_MODE_OUTPUT)

#define LCD_SCL_INPUT       gpio_set_direction(LCD_SCL, GPIO_MODE_INPUT)
#define LCD_SDA_INPUT       gpio_set_direction(LCD_SDA, GPIO_MODE_INPUT)

#define LCD_SDASET         LCD_SDA_OUTPUT; gpio_set_level(LCD_SDA, 1)
#define LCD_SDACLR          LCD_SDA_OUTPUT;gpio_set_level(LCD_SDA, 0)

#define LCD_SCLSET          gpio_set_level(LCD_SCL, 1)
#define LCD_SCLCLR          gpio_set_level(LCD_SCL, 0)


#define I2C_MASTER_NUM     0
#define I2C_MASTER_SDA_IO  LCD_SCL
#define I2C_MASTER_SCL_IO  LCD_SDA
#define I2C_MASTER_FREQ_HZ 100000

#define WRITE_BIT      I2C_MASTER_WRITE         /*!< I2C master write */
#define READ_BIT       I2C_MASTER_READ          /*!< I2C master read */
#define ACK_CHECK_EN   0x1                      /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS  0x0                      /*!< I2C master will not check ack from slave */
#define ACK_VAL        0x0                      /*!< I2C ack value */
#define NACK_VAL       0x1                      /*!< I2C nack value */

typedef enum {
    ht16c23_CMD_IOOUT = 0x80, /*!< Command to set  as configuration Display Data RAM*/
    ht16c23_CMD_DRIMO = 0x82, /*!< Command to set  as Configuration device mode*/
    ht16c23_CMD_SYSMO = 0x84, /*!< Command to set  as Configuration system mode*/
    ht16c23_CMD_FRAME = 0x86, /*!< Command to set  as Configuration frame frequency*/
    ht16c23_CMD_BLINK = 0x88, /*!< Command to set  as Configuration blinking frequency*/
    ht16c23_CMD_IVA   = 0x8a,   /*!< Command to set  as Configuration internal voltage adjustment (IVA) setting*/
} ht16c23_cmd_t;

typedef enum {
    ht16c23_4DUTY_3BIAS = 0x00, /*!< The drive mode 1/4 duty output and 1/3 bias is selected.*/
    ht16c23_4DUTY_4BIAS = 0x01, /*!< The drive mode 1/4 duty output and 1/4 bias is selected.*/
    ht16c23_8DUTY_3BIAS = 0x02, /*!< The drive mode 1/8 duty output and 1/3 bias is selected.*/
    ht16c23_8DUTY_4BIAS = 0x03, /*!< The drive mode 1/8 duty output and 1/4 bias is selected.*/
} ht16c23_duty_bias_t;

typedef enum {
    ht16c23_OSCILLATOR_OFF_DISPLAY_OFF = 0x00, /*!< Display off and disable the internal system oscillator.*/
    ht16c23_OSCILLATOR_ON_DISPLAY_OFF = 0x02, /*!< Display off and enable the internal system oscillator.*/
    ht16c23_OSCILLATOR_ON_DISPLAY_ON = 0x03, /*!< Display on and enable the internal system oscillator.*/
} ht16c23_oscillator_display_t;

typedef enum {
    ht16c23_FRAME_80HZ = 0x00, /*!< Frame frequency is set to 80Hz.*/
    ht16c23_FRAME_160HZ = 0x01, /*!< Frame frequency is set to 160Hz.*/
} ht16c23_frame_frequency_t;

typedef enum {
    ht16c23_BLINKING_OFF = 0x00, /*!< Blinking function is switched off.*/
    ht16c23_BLINKING_2HZ = 0x01, /*!< Blinking function is set to 2HZ.*/
    ht16c23_BLINKING_1HZ = 0x02, /*!< Blinking function is set to 1HZ.*/
    ht16c23_BLINKING_5HZ = 0x03, /*!< Blinking function is set to 0.5HZ.*/
} ht16c23_blinking_frequency_t;

typedef enum {
    ht16c23_VLCD_PIN_VOL_ADJ_OFF = 0x00, /*!<  The Segment/VLCD pin is set as the VLCD pin. Disable the internal voltage adjustment function
        One external resister must be connected between VLCD pin and VDD pin to determine the bias voltage,and internal voltage follower (OP4)
        must be enabled by setting the DA3~DA0 bits as the value other than “0000”. If the VLCD pin is connected to the VDD pin, the internal
        voltage follower (OP4) must be disabled by setting the DA3~DA0 bits as “0000”..*/
    ht16c23_VLCD_PIN_VOL_ADJ_ON = 0x10,/*!< The Segment/VLCD pin is set as the VLCD pin. Enable the internal voltage adjustment function.
        The VLCD pin is an output pin of which the voltage can be detected by the external MCU host.*/
    ht16c23_SEGMENT_PIN_VOL_ADJ_OFF = 0x20, /*!< The Segment/VLCD pin is set as the Segment pin. Disable the internal voltage adjustment function.
        The bias voltage is supplied by the internal VDD power.The internal voltage-follower (OP4) is disabled automatically and DA3~DA0 don’t care.*/
    ht16c23_SEGMENT_PIN_VOL_ADJ_ON = 0x30,/*!<The Segment/VLCD pin is set as the Segment pin. Enable the internal voltage adjustment function.*/
} ht16c23_pin_and_voltage_t;

typedef struct config {
    ht16c23_duty_bias_t duty_bias;
    ht16c23_oscillator_display_t oscillator_display;
    ht16c23_frame_frequency_t frame_frequency;
    ht16c23_blinking_frequency_t blinking_frequency;
    ht16c23_pin_and_voltage_t pin_and_voltage;
    uint8_t  adjustment_voltage; //Range 0x00 to 0x0F
} ht16c23_config_t;

extern ht16c23_config_t ht16c23_conf;
esp_err_t i2c_master_init(void);
esp_err_t iot_ht16c23_ram_write(uint8_t address, uint8_t *buf, uint8_t len);
esp_err_t i2c_bootup_cmd();
esp_err_t iot_ht16c23_init(ht16c23_config_t*  ht16c21_conf);
esp_err_t i2c_master_send_cmd(ht16c23_cmd_t hd16c23_cmd, uint8_t val);


void ht16c23_delay_us(int time);
char ht16c23_write(char skip ,char device_add, char reg_add, char data);
char ht16c23_ram_write(char skip ,char device_add, char reg_add,char seg, char* data);
char ht16c23_read(char skip,char device_add,char reg_add);
char ht16c23_tx(char x);

// ht16c23_start function definition
void ht16c23_start(void);

// ht16c23_stop function definition
void ht16c23_stop(void);

 
//receive function definition
char ht16c23_rx();
void ht16c23_init(void);

void set_segment(int column,char row,char value);
void show_rtc_time(char* day,char* hh,char* mm, char mode);
void show_curve(char* left,char* right);
void show_alarm_time(char* hh,char* mm, char mode);
void write_FND(int FND_ID,char value);
void write_Temperature(char* value);
void show_humidity(char* humidity);
void show_Ble_status(char* value);
void show_battery(char* value);
void show_date(char* date);
void write_BoxId(char* value);
void Set_A(int FND_ID,char value);
void Set_B(int FND_ID,char value);
void Set_C(int FND_ID,char value);
void Set_D(int FND_ID,char value);
void Set_E(int FND_ID,char value);
void Set_F(int FND_ID,char value);
void Set_G(int FND_ID,char value);
void Set_H(int FND_ID,char value);
void Set_I(int FND_ID,char value);
void Set_J(int FND_ID,char value);
void Set_K(int FND_ID,char value);
void Set_L(int FND_ID,char value);
void Set_M(int FND_ID,char value);
void Set_N(int FND_ID,char value);
void Set_O(int FND_ID,char value);
void Set_P(int FND_ID,char value);
void Set_Q(int FND_ID,char value);

void handle_display();


#endif