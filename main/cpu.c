#include "header.h"
#include "driver/gpio.h"

uint8_t msec_count = 0;
uint8_t sec_count = 0;
uint8_t ble_based_decision_cnt = 0;

uint8_t force_sleep_cnt = 0;
uint8_t display_timeout_cnt = 0;
uint8_t backlight_timout_cnt = 0;
volatile uint32_t buzzer_cnt=0;
uint8_t  g_motion_not_occure_cnt = 0;
extern int g_sound_index_value;
void button_gpio_init()
{
    esp_rom_gpio_pad_select_gpio(ALARM_SWITCH); gpio_set_direction(ALARM_SWITCH, GPIO_MODE_INPUT);gpio_pullup_en(ALARM_SWITCH);
    esp_rom_gpio_pad_select_gpio(MODE_SWITCH); gpio_set_direction(MODE_SWITCH, GPIO_MODE_INPUT);gpio_pullup_en(MODE_SWITCH);
    esp_rom_gpio_pad_select_gpio(PWR_OFF_ON_SWITCH); gpio_set_direction(PWR_OFF_ON_SWITCH, GPIO_MODE_INPUT);gpio_pullup_en(PWR_OFF_ON_SWITCH);
}

void gpio_configuration_init()
{
    // row 
    esp_rom_gpio_pad_select_gpio(SWITCH_A); gpio_set_direction(SWITCH_A, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(SWITCH_B); gpio_set_direction(SWITCH_B, GPIO_MODE_OUTPUT); 
    esp_rom_gpio_pad_select_gpio(SWITCH_C); gpio_set_direction(SWITCH_C, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(SWITCH_D); gpio_set_direction(SWITCH_D, GPIO_MODE_OUTPUT);

    // column
    esp_rom_gpio_pad_select_gpio(SWITCH_1); gpio_set_direction(SWITCH_1, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_1);
    esp_rom_gpio_pad_select_gpio(SWITCH_2); gpio_set_direction(SWITCH_2, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_2);
    esp_rom_gpio_pad_select_gpio(SWITCH_3); gpio_set_direction(SWITCH_3, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_3);
    esp_rom_gpio_pad_select_gpio(SWITCH_4); gpio_set_direction(SWITCH_4, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_4);
    esp_rom_gpio_pad_select_gpio(SWITCH_5); gpio_set_direction(SWITCH_5, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_5);
    esp_rom_gpio_pad_select_gpio(SWITCH_6); gpio_set_direction(SWITCH_6, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_6);
    esp_rom_gpio_pad_select_gpio(SWITCH_7); gpio_set_direction(SWITCH_7, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_7);

    //buzzer
    esp_rom_gpio_pad_select_gpio(BACKLIGHT); gpio_set_direction(BACKLIGHT, GPIO_MODE_OUTPUT);

    //charging control
    esp_rom_gpio_pad_select_gpio(CHRG_CTRL); gpio_set_direction(CHRG_CTRL, GPIO_MODE_OUTPUT);

    //SWITCH
    button_gpio_init();


    // 74HC595
    esp_rom_gpio_pad_select_gpio(PWR_EN_595); gpio_set_direction(PWR_EN_595 , GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(DS); gpio_set_direction(DS , GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(STCP); gpio_set_direction(STCP, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(STCP1); gpio_set_direction(STCP1, GPIO_MODE_OUTPUT);
    esp_rom_gpio_pad_select_gpio(SHCP); gpio_set_direction(SHCP, GPIO_MODE_OUTPUT);
    Disable_595_power();
    // BUZZER_OFF;

    // SWITCH
    esp_rom_gpio_pad_select_gpio(REFILL_SWITCH); gpio_set_direction(REFILL_SWITCH, GPIO_MODE_INPUT);//gpio_set_level(REFILL_SWITCH,0);gpio_hold_en(REFILL_SWITCH);
    esp_rom_gpio_pad_select_gpio(BOTTLE_SWITCH); gpio_set_direction(BOTTLE_SWITCH, GPIO_MODE_INPUT);//gpio_set_level(BOTTLE_SWITCH,0);gpio_hold_en(BOTTLE_SWITCH);

    //RTC
    rtcc_gpio_init();
    CHARGING_ENABLE;
}

void adc_configuration_init()
{
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    if (unit == ADC_UNIT_1) 
    {
        adc1_config_width(width);
        adc1_config_channel_atten(channel, atten);
    }
    esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
} 

bool IRAM_ATTR PB_TIMER_ISR(void *args)      // 100 millisecond timer
{
    if(g_Buzzer_toggle_interval != 0)
	{
        buzzer_cnt++;
        if(buzzer_cnt>= g_Buzzer_Threshold)
        {
            buzzer_cnt=0; g_Buzzer_toggle_interval=0;
        }
    }
    else
    {
        buzzer_cnt = 0;
    }

    if(g_led_toggle_state == 1)
	{
        g_led_toggle_cnt++;
        if(g_led_toggle_cnt>= g_led_toggle_threshold)
        {
            g_led_toggle_cnt=0; g_led_toggle_state=2; 
        }
    }
    else
    {
        g_led_toggle_cnt = 0;
    }

    if(f.g_reset_counter_and_flag_F == 1)
    {
        f.g_reset_counter_and_flag_F = 0;
        ble_based_decision_cnt = 0;
        display_timeout_cnt = 0;
        backlight_timout_cnt = 0;
        force_sleep_cnt = 0;
        f.g_display_Off_F = 0;
        f.g_backlight_Off_F = 0;
        f.g_force_sleep_F = 0;
        f.g_bleDisconnection_F = 0;
    }
    
    msec_count++;
    if(msec_count >= 10)
    {
        ble_based_decision_cnt++;
        display_timeout_cnt++;
        backlight_timout_cnt++;

        
        if(++force_sleep_cnt >= MAX3(eeprom.backlight_Off_timeout_sec,eeprom.ble_connection_timeout_sec,eeprom.display_timeout_sec))
        {
            force_sleep_cnt = 0;
            f.g_force_sleep_F = 1;
        }
        if(MIN3(eeprom.backlight_Off_timeout_sec,eeprom.ble_connection_timeout_sec,eeprom.display_timeout_sec) == 0)
        {
            force_sleep_cnt = 0;
            f.g_force_sleep_F = 0;
        }
        // ESP_DRAM_LOGI("force_sleep_cnt",  "%d ==== %d",force_sleep_cnt, MAX3(eeprom.backlight_Off_timeout_sec,eeprom.ble_connection_timeout_sec,eeprom.display_timeout_sec));
        if(eeprom.backlight_Off_timeout_sec != 0)
        {
            // ESP_DRAM_LOGI("BACKLIGHT",  "%d ==== %d",backlight_timout_cnt, eeprom.backlight_Off_timeout_sec);
            if((backlight_timout_cnt % eeprom.backlight_Off_timeout_sec ) == 0) 
            {
                backlight_timout_cnt = 0;
                f.g_backlight_Off_F = 1;
            }
        }
        
        if(eeprom.ble_connection_timeout_sec != 0)
        {
            // ESP_DRAM_LOGI("BLE CONNECTION",  "%d ==== %d",ble_based_decision_cnt, eeprom.ble_connection_timeout_sec);
            if((ble_based_decision_cnt % eeprom.ble_connection_timeout_sec  ) == 0)
            {
                f.g_bleDisconnection_F = 1;
                ble_based_decision_cnt = 0;
            }
        }

        if(eeprom.display_timeout_sec != 0)
        {
            // ESP_DRAM_LOGI("DISPLAY",  "%d ==== %d",display_timeout_cnt, eeprom.display_timeout_sec);
            if((display_timeout_cnt % eeprom.display_timeout_sec) == 0)
            {
                f.g_display_Off_F = 1;
                display_timeout_cnt = 0;
            }
        }
        
        msec_count = 0;
        g_motion_not_occure_cnt++;
        sec_count++;                 f.g_per_sec_F = 1;
        if((sec_count % 4) == 0)    {f.g_sense_key_F = 1;}
        if((sec_count % 10) == 0)   {f.g_10sec_F = 1;    }
        if((sec_count % 30) == 0)   {f.g_30sec_F = 1;    }
        if((sec_count % 40) == 0)   {f.g_40sec_F = 1;    }
        if((sec_count % 50) == 0)   {f.g_50sec_F = 1;    }
        if((sec_count % 60) == 0)   {f.g_per_min_F = 1;  }
        if(sec_count >= 60)
        {
            sec_count = 0;
            
        }
    }
    return pdFALSE; // return whether we need to yield at the end of ISR
}

void timer_configuration_init()          // 100 millisecond timer
{
    int group=0,timer=0,timer_interval_msec=100;
    bool auto_reload=true;
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    }; 
    timer_init(group, timer, &config);
    timer_set_counter_value(group, timer, 0);
    timer_set_alarm_value(group, timer, timer_interval_msec * (TIMER_SCALE/1000));
    timer_enable_intr(group, timer);
    timer_isr_callback_add(group, timer, PB_TIMER_ISR, 0, 0);
    timer_start(group, timer);
}

void FR_check()
{
    if(eeprom.FR_BYTE != INIT_FR)
    {
        ESP_LOGI("FR","Set all the Default values ");
        eeprom.FR_BYTE = INIT_FR;
        eeprom.Device_MGNT_Polar = PCBA_POLAR;
        // sprintf((char *)eeprom.device_name,"%s", "PB");
        remove_all_bonded_devices();
        char tempp[] = "CFG,Pill-Box,691900,005M,1734206265,000,2,0,060S,030M,060S,080S,060S,090S,20,0,*26,& ";
        Decode_save_data((char*)&tempp[0]);

        //PRECFG,OK_TOLERENCE,PROLONG_THRESHOLD,WARNING_THRESHOLD,NO_OF_TIME_VOICEPLAY,&
        char tempp1[] = "PRECFG,0,120S,015S,5,&";
        handle_presetconfigure_configuration_pkt((char*)&tempp1[0]);
        
        memset((char*)&temp_precfg, -1, sizeof(temp_precfg));
        
    }
}

void PB_INIT(void)
{
    memset((char*)&eeprom, 0x00, sizeof(eeprom));
    memset((char*)model_info, 0x00, sizeof(model_info));
    memcpy(model_info,MODEL, 4);
    ESP_LOGI("MODEL","INFO <<<<%s>>>>", model_info);
    read_eeprom((char *)&eeprom);
    FR_check();
    if((AHT20_Read_Status()&0x18)!=0x18)
    {
        AHT20_INIT();
    }
    Read_and_show_temp_humidity();
    
    gpio_configuration_init();
    ht16c23_init();
    accelerometer_init();
    adc_configuration_init() ;
    timer_configuration_init();
    led_task_init();    
    sense_adc_and_show_battery();
    Bluetooth_init(); 
    sound_task_enable();   
}

void sense_adc_and_show_battery()
{
    int adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) 
    {
        if (unit == ADC_UNIT_1) 
        {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        } 
    }
    adc_reading /= NO_OF_SAMPLES;
    uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    voltage_mv *= 2;       //  we Multiply by 2 because of voltage_mv devider 
    PB_reading.battery_percentage = ((voltage_mv - 3000));
    ESP_LOGI("VBAT", "%ld, %d",voltage_mv, PB_reading.battery_percentage );
    PB_reading.battery_percentage /= 12;
    ESP_LOGI("VBAT", "%ld, %d",voltage_mv, PB_reading.battery_percentage );
     if(PB_reading.battery_percentage<0){PB_reading.battery_percentage=0;}
    if(PB_reading.battery_percentage>100){PB_reading.battery_percentage=100;}
    ESP_LOGI("VBAT", "%ld, %d",voltage_mv, PB_reading.battery_percentage );
    if(PB_reading.battery_percentage < eeprom.battery_alert_threshold)
    {
        f.g_battery_alert_F = 1;
    }
    else
    {
        f.g_battery_alert_F = 0;
    }
    if(voltage_mv > 3000) 
    {
        if((voltage_mv > 3000) && (voltage_mv < 3400))
        {
            show_battery("1");
        }
        if((voltage_mv > 3400)&& (voltage_mv < 3800))
        {
            show_battery("2");
        }
        if(voltage_mv > 3800)
        {
            show_battery("3");
        }
    }
    else
    {
        show_battery("0");
    }
   
    // uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(3800, adc_chars);
    // printf(" Battery Voltage %d, %fmv\n", adc_reading,(float)(voltage_mv)/1000);
}

void wakeup_configuration_init(char mode)
{
    if(mode == 1)
    {
        const uint64_t ext_wakeup_pwr_switch_mask = (1ULL << PWR_OFF_ON_SWITCH);
        esp_rom_gpio_pad_select_gpio(PWR_OFF_ON_SWITCH);
        gpio_set_direction(PWR_OFF_ON_SWITCH, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PWR_OFF_ON_SWITCH, GPIO_PULLUP_ONLY);

        ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext_wakeup_pwr_switch_mask, ESP_EXT1_WAKEUP_ANY_LOW));        
        gpio_hold_en(PWR_OFF_ON_SWITCH);
        gpio_deep_sleep_hold_en();
    }
    else
    {
        const uint64_t ext_wakeup_mode_switch_mask = (1ULL << MODE_SWITCH);
        const uint64_t ext_wakeup_alarm_switch_mask = (1ULL << ALARM_SWITCH);
        const uint64_t ext_wakeup_pwr_switch_mask = (1ULL << PWR_OFF_ON_SWITCH);
        const uint64_t ext_wakeup_INT_mask = (1ULL << ACC_INT1);

        esp_rom_gpio_pad_select_gpio(MODE_SWITCH);
        gpio_set_direction(MODE_SWITCH, GPIO_MODE_INPUT);
        gpio_set_pull_mode(MODE_SWITCH, GPIO_PULLUP_ONLY);

        esp_rom_gpio_pad_select_gpio(ALARM_SWITCH);
        gpio_set_direction(ALARM_SWITCH, GPIO_MODE_INPUT);
        gpio_set_pull_mode(ALARM_SWITCH, GPIO_PULLUP_ONLY);

        esp_rom_gpio_pad_select_gpio(PWR_OFF_ON_SWITCH);
        gpio_set_direction(PWR_OFF_ON_SWITCH, GPIO_MODE_INPUT);
        gpio_set_pull_mode(PWR_OFF_ON_SWITCH, GPIO_PULLUP_ONLY);

        ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup(ext_wakeup_mode_switch_mask | ext_wakeup_alarm_switch_mask | ext_wakeup_pwr_switch_mask | ext_wakeup_INT_mask, ESP_EXT1_WAKEUP_ANY_LOW));
        gpio_hold_en(MODE_SWITCH);
        gpio_hold_en(ALARM_SWITCH);
        gpio_hold_en(PWR_OFF_ON_SWITCH);
        gpio_deep_sleep_hold_en();
    }
    
}

void enter_into_sleep_mode(uint32_t timeout)
{
    esp_sleep_enable_timer_wakeup(((timeout) * 1000000) - 100);  // wakeup from sleep before 100ms
    printf("Entering into the Deep sleep mode for %ld Sec\n",timeout);
    sleep_mode_start();
}

void sleep_mode_start()
{
    printf("Device entering in sleep mode...\n");
    esp_deep_sleep_start();
}

void sleep_handle(char mode)
{
    
    // motion not occure  from last 30 sec and bluetoothh is not connected then we will go into the sleep 
    if(f.g_motionsense_F == 1)
    {
        f.g_motionsense_F = 0;
        g_motion_not_occure_cnt = 0;
    }
    if(mode == 0)
    {
        Is_ble_connected = false;
        g_motion_not_occure_cnt = 30;
    }
    if(Is_ble_connected == true)g_motion_not_occure_cnt=0;
    // printf("BLE = %d,cnt =%d\n",(int)Is_ble_connected,g_motion_not_occure_cnt);
    if((Is_ble_connected == false) && (g_motion_not_occure_cnt == 30) && ((g_sound_index_value == -1)))
    {
        char current_alarm[MAXIMUM_SUPPORTED_TIMER] = {0}, next_alarm = 0; uint32_t l_epoch = 0;uint16_t next_compartment;	
	    /*Get All Timer_alarm lies between the epoch window, if not a single timer lies in the epoch windoe then find the next nearest timer LP, and current epoch value*/
	    int32_t sleep_time = Find_next_alarm_and_current_epoch((char*)&current_alarm[0], (char*)&next_alarm, &l_epoch,&next_compartment);
        if((sleep_time<5) && (sleep_time !=-1))
        {
            return;
        }
        wakeup_configuration_init(0);
        accelerometer_init();  
        hold_required_gpios(); 
        if(sleep_time!=-1)
        {enter_into_sleep_mode((sleep_time-3)); }
        else{
            enter_into_sleep_mode(SLEEP_INTERVAL_SEC); 
        }   //  300 Sec 
    }
}

void hold_required_gpios()
{
    //RGB_Buzzer_Control(0,0,0,0); 
    Disable_595_power();  
    BACKLIGHT_OFF;
    gpio_set_level(SWITCH_A,1);
    gpio_set_level(SWITCH_B,1);
    gpio_set_level(SWITCH_C,1);
    gpio_set_level(SWITCH_D,1);
    gpio_set_level(CHRG_CTRL,0);
    gpio_set_level(AUDIO_IN,0);
    gpio_set_level(AUDIO_BCLK,0);
    gpio_set_level(AUDIO_LRCLK,0);

    gpio_hold_en((gpio_num_t) SHCP);
    gpio_hold_en((gpio_num_t) STCP);
    gpio_hold_en((gpio_num_t) DS);
    gpio_hold_en((gpio_num_t) STCP1);
    gpio_hold_en((gpio_num_t) BACKLIGHT);
    gpio_hold_en((gpio_num_t) CHRG_CTRL);
    gpio_hold_en((gpio_num_t)PWR_EN_595);
    gpio_hold_en((gpio_num_t) SWITCH_A);
    gpio_hold_en((gpio_num_t) SWITCH_B);
    gpio_hold_en((gpio_num_t) SWITCH_C);
    gpio_hold_en((gpio_num_t) SWITCH_D);
    gpio_hold_en((gpio_num_t) AUDIO_IN);
    gpio_hold_en((gpio_num_t) AUDIO_BCLK);
    gpio_hold_en((gpio_num_t) AUDIO_LRCLK);
}

uint8_t Show_reset_reason()
{
    char reset_reason = esp_reset_reason();
    ESP_LOGI("", "\r\n");
    switch (reset_reason)
    {
        case ESP_RST_UNKNOWN:
            ESP_LOGI("BOOT", "Reset reason can not be determined");
            break;
        case ESP_RST_POWERON:
            ESP_LOGI("BOOT", "Reset due to power-on event");
            break;
        case ESP_RST_EXT:
            ESP_LOGI("BOOT", "Reset by external pin");
            break;
        case ESP_RST_SW:
            ESP_LOGI("BOOT", "Software reset via esp_restart");
            break;
        case ESP_RST_PANIC:
            ESP_LOGI("BOOT", "Software reset due to exception/panic");
            break;
        case ESP_RST_INT_WDT:
            ESP_LOGI("BOOT", "Reset (software or hardware) due to interrupt watchdog");
            break;
        case ESP_RST_TASK_WDT:
            ESP_LOGI("BOOT", "Reset due to task watchdog");
            break;
        case ESP_RST_WDT:
            ESP_LOGI("BOOT", "Reset due to other watchdogs");
            break;
        case ESP_RST_DEEPSLEEP:
            ESP_LOGI("BOOT", "Reset after exiting deep sleep mode");
            break;
        case ESP_RST_BROWNOUT:
            ESP_LOGI("BOOT", "Brownout reset (software or hardware)");
            break;
        case ESP_RST_SDIO:
            ESP_LOGI("BOOT", "Reset over SDIO");
            break;
        
        default:
            break;
    }
    return(reset_reason);
}

void Buzzer_Pattern(uint8_t Pattern_duration)
{
    // uint16_t duration = 0;
    // uint8_t count = 10;
    // if(Pattern_duration == MSEC_PATTERN)
    // {
    //     duration = 100;
    // }
    // else if(Pattern_duration == SEC_PATTERN)
    // {
    //     duration = 1000;
    // }
    // else if(Pattern_duration == NO_PATTERN)
    // {
    //     duration = 200;
    //     count = 1;
    // }
    // for (size_t i = 0; i < count; i++)
    // {
        f.g_buzzer_ON=1;
        vTaskDelay(Pattern_duration/portTICK_PERIOD_MS);
        f.g_buzzer_ON=0;
    //}   
}

uint8_t Show_wakeup_reason()
{
    ESP_LOGI("","\n");
    uint8_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason) 
    {
        case ESP_SLEEP_WAKEUP_EXT0: {
            ESP_LOGI("WAKEUP REASON","Wake up from ext0\n");
            break;
        }

        case ESP_SLEEP_WAKEUP_EXT1: 
        {
            ESP_LOGI("WAKEUP REASON","Wake up from ESP_SLEEP_WAKEUP_EXT1\n");
            break;
        }

        case ESP_SLEEP_WAKEUP_GPIO:
                ESP_LOGI("WAKEUP REASON","Wake up from ESP_SLEEP_WAKEUP_GPIO\n");
            break;
        
        case ESP_SLEEP_WAKEUP_TIMER:
        {
            ESP_LOGI("WAKEUP REASON","Wake up from timer. Time spent in deep sleep\n");
            break;
        }

        default:
            ESP_LOGI("WAKEUP REASON","Not a deep sleep reset\n");

    }
    return(wakeup_reason);
}
 




