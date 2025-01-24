#ifndef GLOBAL_H_
#define GLOBAL_H_

// Firmware and hardware versions
#define FIRMWARE_VERSION "PB28_H3.2_V0.1"
#define HW_VERSION "PB_A_1.3.2"
#define SW_VERSION "H3.2_V0.1"
#define MODEL "PB28"

extern char model_info[10];
// Maximum data capacity
#define MAXIMUM_DATA_CAPACITY 5000

// LED toggle parameters
extern int g_led_toggle_cnt;
extern int g_led_toggle_state;
extern int g_led_toggle_threshold;

// Buzzer parameters
extern char INIT_FR;
extern char g_Buzzer_toggle_interval;
extern volatile int g_Buzzer_Threshold;

// Matched ID index
extern char g_matched_id_index;

// On/Off states
#define OFF 0
#define ON 1

// Maximum supported boxes and timers
#define MAXIMUM_SUPPORTED_BOXES 28
#define MAXIMUM_SUPPORTED_TIMER 121

// Default alarm parameters
#define DEFAULT_ALARM_GAP 5
#define DEFAULT_TOTAL_REPEATITION_CNT 6

// Leap year calculation
#define LEAP_YEAR(year) ((year % 4) == 0)

// Min/Max macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN3(a, b, c) (MIN(MIN(a, b), c))
#define MAX3(a, b, c) (MAX(MAX(a, b), c))

// Device polarity enumeration
typedef enum {
    PCBA_POLAR = 0,
    CABINET_POLAR,
} _DEVICE_POLARITIY;

// Day name enumeration
typedef enum {
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
} _DAY_NAME_ENUM;

// Day time enumeration
typedef enum {
    E_MORNING = 0,
    E_AFTERNOON,
    E_EVENING,
    E_NIGHT,
} _DAY_TIME_ENUM;

// Data structure for events
typedef struct {
    volatile char event;
} _DTA;

// EEPROM address definitions
#define G_SEC 0
#define G_MIN 2
#define G_HH 3

// Configuration alarm time structure
typedef struct {
    char Box_ID;
    char ID_CONFIGURED_STATUS;
    uint32_t timer_epoch;
    uint32_t timer_result_epoch;
    uint16_t buffer_period_epoch;
    char alarm_repetition_cnt;
    uint16_t total_alarm_duratition_epoch;
    uint16_t alarm_gap_epoch;
    uint32_t last_triggred_time;
} _CONFIG_ALARM_TIME;

// Memory parameter structure
typedef struct {
    uint32_t max_memory_capacity;
    uint32_t memory_unsend_data_cnt;
} _MEMORY_PARAM;

extern volatile _MEMORY_PARAM g_memory_parameter;

// Wrong action structure
typedef struct {
    char box_cnt;
    uint32_t epoch[10];
} _wrong_action;

// Pre-configuration structure
typedef struct {
    uint8_t temperature_unit;
    uint8_t prolong_threshold;
    uint8_t prolong_threshold_unit;
    uint32_t prolong_threshold_sec;
    uint8_t warning_threshold;
    uint8_t warning_threshold_unit;
    uint32_t warning_threshold_sec;
    uint8_t no_of_time_voiceplay;
} _PRECFG;

// Warning threshold structure
typedef struct {
    int32_t start_time;
    int32_t last_time;
} _warning_threshold;

// Pre-configuration threshold structure
typedef struct {
    _warning_threshold warn_th[MAXIMUM_SUPPORTED_BOXES];
} _precfg_threshold;

// Prolonged data structure
typedef struct {
    char prolong_cnt;
    uint32_t prolonged_epoch[10];
} _prolonged_data;

// EEPROM structure
typedef struct {
    char FR_BYTE;
    char PWR_ON_OFF;
    char last_alarm_id;
    char alarm_volume;
    char REPEATITION_DAYS;
    char device_name[20];
    char time_mode;
    uint8_t ble_connection_timeout;
    uint8_t ble_connection_timeout_unit;
    uint32_t ble_connection_timeout_sec;
    uint8_t display_timeout;
    uint8_t display_timeout_unit;
    uint32_t display_timeout_sec;
    uint8_t backlight_Off_timeout;
    uint8_t backlight_Off_timeout_unit;
    uint32_t backlight_Off_timeout_sec;
    uint8_t battery_alert_threshold;
    uint8_t Device_MGNT_Polar;
    uint8_t alarm_gap_unit;
    uint8_t ping_interval_unit;
    uint8_t TADE_interval_unit;
    unsigned char total_timers_configured;
    uint8_t panic_interval_unit;
    _PRECFG pre_cfg;
    _CONFIG_ALARM_TIME config_alarm_time[MAXIMUM_SUPPORTED_TIMER];
    _wrong_action wrong_action[MAXIMUM_SUPPORTED_TIMER];
    _prolonged_data prolong_action[MAXIMUM_SUPPORTED_TIMER];
    int alarm_gap;
    int alarm_gap_sec;
    int TADE_interval;
    int TADE_interval_sec;
    int panic_interval;
    int panic_interval_sec;
    int ping_interval;
    int ping_interval_sec;
    int BLE_PASS;
    uint32_t rtc_epoch;
} _eeprom;

extern volatile _eeprom eeprom;

// Flags structure
typedef struct {
    volatile char g_default_config_F : 1;
    volatile char g_change_device_name_pass_F : 2;
    volatile char g_buzzer_ON : 1;
    volatile char g_buzzer_independent_ON : 1;
    volatile char g_audio_ON : 1;
    volatile char g_motionsense_F : 1;
    volatile char g_per_sec_F : 1;
    volatile char g_sense_key_F : 1;
    volatile char g_30sec_F : 1;
    volatile char g_10sec_F : 1;
    volatile char g_50sec_F : 1;
    volatile char g_40sec_F : 1;
    volatile char g_per_min_F : 1;
    volatile char g_lcd_updated_F : 1;
    volatile char g_ble_rcvd_F : 1;
    volatile char g_led_buffer_busy_F : 1;
    volatile char g_panic_alert_F : 1;
    volatile char g_battery_alert_F : 1;
    volatile char g_backlight_Off_F : 1;
    volatile char g_display_Off_F : 1;
    volatile char g_bleDisconnection_F : 1;
    volatile char g_force_sleep_F : 1;
    volatile char g_reset_counter_and_flag_F : 1;
} _flags;

extern volatile _flags f;

// Function prototypes
char PB_immediate_restart();
char PB_esp_restart();
char ascii2hex(char l_char);
uint32_t convert_time_date_into_epoch(char* date, char* time);
void convert_epoch_into_gmttime_and_date(time_t epoch, char* date, char* time);
char get_parameter_from_data(char *out, char *data, char *delimiter, char number, char *const_strng, char *termination, int exact_len, int max_len);

#endif // GLOBAL_H_
