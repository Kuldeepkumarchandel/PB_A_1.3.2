#ifndef LED_H_
#define LED_H_

extern uint8_t led_matrix[8];
#define LOW_SIDE_595     2 // U8
#define HIGH_SIDE_595    1 // U11
void LED_MATRIX_SET();
void LED_MATRIX_RESET();
void wellcome_greetings();
void Run_LED_Matrix(char* led_matrix);
void LED28_Control(int column,int row,char value);
void RGB_Buzzer_Control(int8_t RED,int8_t GREEN,int8_t BLUE,int8_t Buzzer);
void Run_Buzzer();
void led_task_init();
void led_task_manager();
void LED28_Control(int column,int row,char value);
void Blow_Buzzer(int off_on_interval_ms, int timeout_in_ms);
void Stop_Buzzer();
void play_panic_buzzer(void);
#endif