#include "header.h"

//  Global Variables
char week_number[3] = {0};
char alarm_number[2] = {0}; 
char total_53weeks_available = 0;
char reset_reason = 0, just_pwr_on = 0;

void unhold_holded_gpios()
{
    gpio_hold_dis((gpio_num_t) ALARM_SWITCH);     
    gpio_hold_dis((gpio_num_t) MODE_SWITCH);
    gpio_hold_dis((gpio_num_t) PWR_OFF_ON_SWITCH);
    gpio_hold_dis((gpio_num_t) SHCP);
    gpio_hold_dis((gpio_num_t) STCP);
    gpio_hold_dis((gpio_num_t) DS);
    gpio_hold_dis((gpio_num_t) STCP1);
    gpio_hold_dis((gpio_num_t) BACKLIGHT);
    gpio_hold_dis((gpio_num_t) CHRG_CTRL);
    gpio_hold_dis((gpio_num_t) PWR_EN_595);
    gpio_hold_dis((gpio_num_t) SWITCH_A);
    gpio_hold_dis((gpio_num_t) SWITCH_B);
    gpio_hold_dis((gpio_num_t) SWITCH_C);
    gpio_hold_dis((gpio_num_t) SWITCH_D);
    gpio_hold_dis((gpio_num_t) AUDIO_IN);
    gpio_hold_dis((gpio_num_t) AUDIO_BCLK);
    gpio_hold_dis((gpio_num_t) AUDIO_LRCLK);
}

void sleep_mode_handling()
{ 
    char wakeup_reason = 0;
    unhold_holded_gpios();
    
    // Configure GPIO pins
    esp_rom_gpio_pad_select_gpio(PWR_EN_595); 
    gpio_set_direction(PWR_EN_595, GPIO_MODE_OUTPUT); 
    Disable_595_power();
    
    esp_rom_gpio_pad_select_gpio(BACKLIGHT); 
    gpio_set_direction(BACKLIGHT, GPIO_MODE_OUTPUT);
    gpio_set_level(BACKLIGHT, 0);
    
    esp_rom_gpio_pad_select_gpio(SWITCH_A); 
    gpio_set_direction(SWITCH_A, GPIO_MODE_OUTPUT);
    gpio_set_level(SWITCH_A, 1);
    
    esp_rom_gpio_pad_select_gpio(SWITCH_B); 
    gpio_set_direction(SWITCH_B, GPIO_MODE_OUTPUT);
    gpio_set_level(SWITCH_B, 1);
    
    esp_rom_gpio_pad_select_gpio(SWITCH_C); 
    gpio_set_direction(SWITCH_C, GPIO_MODE_OUTPUT);
    gpio_set_level(SWITCH_C, 1);
    
    esp_rom_gpio_pad_select_gpio(SWITCH_D); 
    gpio_set_direction(SWITCH_D, GPIO_MODE_OUTPUT);
    gpio_set_level(SWITCH_D, 1);
    
    button_gpio_init();

    ESP_LOGI("BBBBBBBBBBB", "");    
    wakeup_reason = Show_wakeup_reason();
    ESP_LOGI("AAAAAAAAAA", "%d", wakeup_reason);
    
    if ((wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) || (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1))
    {      
        ESP_LOGI("DDDDDDDDDDDDD", "");  
        rtcc_gpio_init(); 
        accelerometer_init();  
        
        // Configure column pins
        esp_rom_gpio_pad_select_gpio(SWITCH_1); gpio_set_direction(SWITCH_1, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_1);
        esp_rom_gpio_pad_select_gpio(SWITCH_2); gpio_set_direction(SWITCH_2, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_2);
        esp_rom_gpio_pad_select_gpio(SWITCH_3); gpio_set_direction(SWITCH_3, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_3);
        esp_rom_gpio_pad_select_gpio(SWITCH_4); gpio_set_direction(SWITCH_4, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_4);
        esp_rom_gpio_pad_select_gpio(SWITCH_5); gpio_set_direction(SWITCH_5, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_5);
        esp_rom_gpio_pad_select_gpio(SWITCH_6); gpio_set_direction(SWITCH_6, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_6);
        esp_rom_gpio_pad_select_gpio(SWITCH_7); gpio_set_direction(SWITCH_7, GPIO_MODE_INPUT); gpio_pullup_en(SWITCH_7);
     
        if (!((Read_clock_and_compare_with_alarm_epoch(0) == 1) || 
              (sense_button_status_polling(2) == 1) || 
              (Keypad_Scan(0) == 1)))
        {
            sleep_handle(0);
        }        
    } 
}

void Dead_battery_handling()
{
    adc_configuration_init();    
    sense_adc_and_show_battery();
    ESP_LOGI("PB INIT ", "PB_reading.battery_percentage=%d", PB_reading.battery_percentage); 
    
    if (PB_reading.battery_percentage < 20)
    {
        ESP_LOGI("DEAD BATTERY ", "PILL-BOX IS GOING INTO SLEEP MODE"); 
        sleep_handle(0);
    }
}

SemaphoreHandle_t g_mutex_Memory = NULL;

char GetMemoryMutex(void)
{
    if (xSemaphoreTake(g_mutex_Memory, 2000) == pdPASS)
    {
        return 1;
    }
    else
    {
        PB_esp_restart();  
    }
    return 0;  
}

char Release_Memory_mutex(void)
{
    if (xSemaphoreGive(g_mutex_Memory))
    {
        return 1;
    }
    return 0;
}

char power_off_on_handling(void)
{
    memset((char*)&eeprom, 0x00, sizeof(eeprom));
    read_eeprom((char *)&eeprom);
    ESP_LOGI("AAAAAAAAa", "");
    
    if (eeprom.PWR_ON_OFF == 0)
    {
        wakeup_configuration_init(1); 
        hold_required_gpios();
        ESP_LOGI("11111111111", "");
        sense_button_status_polling(30);   // 50
        
        if (eeprom.PWR_ON_OFF == 0)
        {
            enter_into_sleep_mode(SLEEP_INTERVAL_SEC*10);
        }
        else
        {
            ESP_LOGI("11111111111", "");
            return 1;
        }
    }
    ESP_LOGI("000000000000", "");
    return 0;
}

void read_battery_level()
{
    static char cnt = 19;
    cnt++;
    
    if (cnt == 18)
    {
        CHARGING_DISABLE; 
    }
    else if (cnt == 19)
    {
        cnt = 0;
        sense_adc_and_show_battery(); 
        CHARGING_ENABLE;  
    }
    else if (cnt == 20)
    {
        cnt = 0;
        CHARGING_DISABLE; 
        vTaskDelay(1000/portTICK_PERIOD_MS);
        sense_adc_and_show_battery(); 
        CHARGING_ENABLE;  
    }
}

void app_main(void)
{
   
    reset_reason = Show_reset_reason();
    g_mutex_Memory = xSemaphoreCreateMutex();
    
    if (g_mutex_Memory != NULL)
    {
        printf("successfully created mutex, reset reason = %d\n", (int)reset_reason);
    }
    else
    {
        PB_esp_restart();
    }

    unhold_holded_gpios();
    just_pwr_on = power_off_on_handling();
    unhold_holded_gpios();
    
    if (!((reset_reason == ESP_RST_POWERON) || (just_pwr_on == 1)))
    {
        sleep_mode_handling();
    }
    
    ESP_LOGI("PB INIT ", "Done"); 
    PB_INIT();
    // ONLY FOR TESTING BLE DATA
    char tempp[] = "CFG,Pill-Box,691900,005M,1727867270,003,00-1727867275,14-1727867280,24-1727867290,2,0,060S,030M,060S,080S,060S,090S,20,0,*12,&";
    // char tempp[] = "CFG,Pill-Box,691900,005M,1734206164,000,2,0,060S,030M,060S,080S,060S,090S,20,0,*12,&";
    Decode_save_data((char*)&tempp[0]);
    // fatch_data(tempp);

    if ((reset_reason == ESP_RST_POWERON) || (just_pwr_on == 1))
    {
        wellcome_greetings();
    } 
    printf("**********  Device Version[HW: %s SW: %s, Firmware: %s]  **********\n", 
           HW_VERSION, SW_VERSION, FIRMWARE_VERSION);
   
    RGB_Buzzer_Control(0, 0, 0, 1);
    f.g_per_sec_F = 1;
    f.g_50sec_F = 1;
    f.g_10sec_F = 1;

    show_curve("1", "1");
    f.g_change_device_name_pass_F = 0;
    
    while (1)
    {           
        if ((f.g_change_device_name_pass_F != 0) && (GetBluetooth_ConnectionStatus() == false))
        {
            if (f.g_change_device_name_pass_F == 2)
            {
                ESP_LOGI(".", "Restart due to Device Password change");
                remove_all_bonded_devices();
            }
            else
            {
                ESP_LOGI(".", "Restart due to Device Name change");
            }
            vTaskDelay(3000/portTICK_PERIOD_MS);
            f.g_change_device_name_pass_F = 0;
            PB_immediate_restart();
        }
        
        if (f.g_50sec_F == 1)
        {
            f.g_50sec_F = 0;                      
            Read_and_show_temp_humidity();            
        }
        
        if (f.g_bleDisconnection_F == 1)
        {
            f.g_bleDisconnection_F = 0;
            ble_disconnect_connected_device();
        }

        if(f.g_force_sleep_F == 1)
        {
            f.g_force_sleep_F = 0;
            sleep_handle(0);
        }
        
        if (f.g_10sec_F == 1)
        {
            f.g_10sec_F = 0;
            read_battery_level();
            Read_clock_and_compare_with_alarm_epoch(1);
            
            if (f.g_panic_alert_F == 1)
            {
                PB_reading.ALERT_TYPE = A_PANIC;                
            }
            else if (f.g_battery_alert_F == 1)
            {
                PB_reading.ALERT_TYPE = A_BATTERY;
            }
            else
            {
                PB_reading.ALERT_TYPE = A_NONE;
            }
        }
        
        if (f.g_per_sec_F == 1)
        {
            f.g_per_sec_F = 0;
           
            if (Is_ble_connected == false)               // change for advertisement
            {
                Bluetooth_data_advertisement();
            }
            
            if (GetBluetooth_ConnectionStatus())
            {
                show_Ble_status("1");
            }
            else
            {
                show_Ble_status("0");
            }
        }
        
        if (f.g_panic_alert_F == 1)
        {
            ESP_LOGI("PANIC", "STATUS = %d", f.g_panic_alert_F);
            play_panic_buzzer();
        }
        else
        {
            sleep_handle(1);
        }
        
        Handle_BLE_Data();   // config, ota
        handle_display();
        sense_button(1);
        
        if (f.g_sense_key_F == 1)
        {
            f.g_sense_key_F = 0;
            Keypad_Scan(1); 
        }        
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
