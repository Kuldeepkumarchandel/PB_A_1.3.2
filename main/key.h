#ifndef KEY_H_
#define Key_h_

#define  SCANNING_THRESHOLD     20
#define NO_OF_ROW               4
#define NO_OF_COLUMN            7
#define HIGH_SIDE_U11                  1
#define LOW_SIDE_U8                  2

extern volatile _precfg_threshold temp_precfg;
extern char output_array[MAXIMUM_SUPPORTED_BOXES];

void find_key(char column, char row);
char row_handle();
/// @brief  This function is used to configure row value, Pass 0 ==> to set row 0, 1==> set  row 1, 2==> set
char column_handle(int row_value);
void sense_key();
void Keypad_init();
char Keypad_Scan(char mode);
char sense_button(char mode);
char sense_button_status_polling(char threshold);
int validate_occureed_event(uint8_t event_index,uint32_t current_epoch);
// 74HC595
void led_pattern();
void clock_enable_2(void);
void clock_enable_1(void);
void latch_enable(void);
void send_data_on_595(uint8_t data_out,uint8_t IC_number);
int output_array_mapping(uint8_t row , uint8_t column);
void output_array_index_to_row_column_mapping(int index,uint8_t *row , uint8_t *column);
#endif