#ifndef CPU_H_
#define CPU_H_

#define NO_PATTERN              0
#define MSEC_PATTERN            1
#define SEC_PATTERN             2
#define SLEEP_INTERVAL_SEC      120

#define DEFAULT_VREF            1100
#define NO_OF_SAMPLES           1000 

#define TIMER_DIVIDER         (80)
#define TIMER_SCALE           (APB_CLK_FREQ / TIMER_DIVIDER)

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
static const adc_channel_t channel = ADC_CHANNEL_0;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

char GetMemoryMutex(void);
char Release_Memory_mutex(void);
void button_gpio_init();
void handle_LED();
bool IRAM_ATTR PB_TIMER_ISR(void *args);
void timer_configuration_init();
void adc_configuration_init();
void gpio_configuration_init();
void wellcome_greetings();
void sleep_mode_start();
void sleep_handle(char mode);
void enter_into_sleep_mode(uint32_t timeout);
void wakeup_configuration_init(char mode);
void PB_INIT(void);
void sense_adc_and_show_battery();
uint8_t Show_reset_reason();
uint8_t Show_wakeup_reason();
void Buzzer_Pattern(uint8_t Pattern_duration);
void hold_required_gpios();
#endif