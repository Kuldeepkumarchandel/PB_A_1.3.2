#include "header.h"

char output_array[MAXIMUM_SUPPORTED_BOXES] = {0},output_array_pre[MAXIMUM_SUPPORTED_BOXES] = {0};
char limit=1;
volatile _precfg_threshold temp_precfg;

char row_handle()
{
    static char current_row;//,count = 4;//, current_row = 0,last;
    // count++;
    // if(count >=limit)
    // {
    //     count = 0;
        // row selection
        if(current_row  == 0)       { /*ESP_LOGI("TAG", "ROW1_LOW ");*/ ROW1_LOW;   ROW2_HIGH;  ROW3_HIGH;  ROW4_HIGH; current_row = 1;  }//last=0;return 0; }
        else if(current_row  == 1)  { /*ESP_LOGI("TAG", "ROW2_LOW ");*/  ROW2_LOW;   ROW1_HIGH;  ROW3_HIGH;  ROW4_HIGH; current_row = 2; }// last=1;return 1; }
        else if(current_row  == 2)  { /*ESP_LOGI("TAG", "ROW3_LOW ");*/  ROW3_LOW;   ROW1_HIGH;  ROW2_HIGH;  ROW4_HIGH; current_row = 3; }// last=2;return 2; }
        else if(current_row  == 3)  { /*ESP_LOGI("TAG", "ROW4_LOW ");*/  ROW4_LOW;   ROW1_HIGH;  ROW2_HIGH;  ROW3_HIGH; current_row = 0; }// last=3;return 3; }
   // }
    return (current_row);
}

int output_array_mapping(uint8_t row , uint8_t column)// it is for magnet position to 28 byte array (not for led)
{
    if(row != 0){row -= 1;}
    else if(row== 0){row = 3;}

    column = 7-column;

    return (row+column*4);

}

void output_array_index_to_row_column_mapping(int index,uint8_t *row , uint8_t *column)// it is for magnet position to 28 byte array (not for led)
{
    if(index <= 7)
    {
        *row = 1;*column=7-index;
    }
    else if(index<=14)
    {
        *row = 2;*column=14-index;
    }
    else if(index<=21)
    {
        *row = 3;*column=21-index;
    }
    else if(index<=28)
    {
        *row = 4;*column=28-index;
    }
}

char column_handle(int row_value)
{
    // Return value indicating if any switch was sensed
    char sensed = 0;
    char prolonged_sensed = 0;
    // Static variables to maintain state between function calls
    static struct {
        uint8_t columns[7];  // Counter for each column 
        int8_t last_row;     // Track last processed row
        uint8_t bottle_cnt;  // Counter for bottle switch
    } state = {0};

    // Validate input range
    if(row_value < 0 || row_value > 3) {
        return -1;
    }

    // Reset column counters when row changes to avoid false triggers
    if(state.last_row != row_value) {
        state.last_row = row_value;
        memset(state.columns, 0, sizeof(state.columns));
    }

    // Small delay for switch debouncing
    esp_rom_delay_us(50);

    // Temporary storage for LED control
    int8_t led_row;
    // volatile _precfg_threshold temp_precfg;

  
    // Define switch configuration for easier processing
    const struct {
        gpio_num_t pin;      // GPIO pin number
        uint8_t day;         // Day mapping (MON-SUN)
        uint8_t column;      // Column mapping (1-7)
    } switches[] = {
        {SWITCH_1, SUN, 1},
        {SWITCH_2, SAT, 2},
        {SWITCH_3, FRI, 3},
        {SWITCH_4, THU, 4},
        {SWITCH_5, WED, 5},
        {SWITCH_6, TUE, 6},
        {SWITCH_7, MON, 7}
    };

    // Process each switch
    for(int i = 0; i < 7; i++) 
    {
        // Determine active state based on polarity configuration
        bool is_active = (eeprom.Device_MGNT_Polar == CABINET_POLAR) ? gpio_get_level(switches[i].pin) == 1 : gpio_get_level(switches[i].pin) == 0;

        int array_idx = output_array_mapping(row_value, switches[i].column);

        if(is_active) 
        {
            // Increment counter and check if threshold reached
            if(++state.columns[i] >= limit) 
            {
                sensed = 1;
                state.columns[i] = 0;

                uint32_t current_time = esp_log_timestamp()/1000;

                // Initialize or update timing
                if(temp_precfg.warn_th[array_idx].last_time == -1) 
                {
                    temp_precfg.warn_th[array_idx].start_time = temp_precfg.warn_th[array_idx].last_time = current_time;
                    ESP_LOGI("TAG", "Start time: %lu===  %d", temp_precfg.warn_th[array_idx].start_time, array_idx);
                } 
                else 
                {
                    // Calculate time difference based on switch type   
                    ESP_LOGI("TAG", "current_time >>%ld<< last time: %lu", current_time, temp_precfg.warn_th[array_idx].start_time);                 
                    uint32_t time_diff = current_time - temp_precfg.warn_th[array_idx].start_time;
                    ESP_LOGI("TAG", "Switch %d time difference: %lu ms, %ld Sec", i+1, time_diff, eeprom.pre_cfg.warning_threshold_sec);
                    // Check if warning threshold exceeded
                    if(time_diff >= eeprom.pre_cfg.prolong_threshold_sec)  // 120 sec
                    {
                        uint32_t current_epoch = 0;
                        READ_RTC_and_GET_Epoch(&current_epoch);
                        ESP_LOGI("PROLONG", "Prolonged sensed for switch %d, %lu ", i+1, time_diff);
                        if(eeprom.prolong_action[array_idx].prolong_cnt < 10)
                        {
                            eeprom.prolong_action[array_idx].prolonged_epoch[(int)(eeprom.prolong_action[array_idx].prolong_cnt)] = (current_epoch - time_diff);
                        }
                        eeprom.prolong_action[array_idx].prolong_cnt++;
                        
                        prolonged_sensed = 1;
                    }
                    if(time_diff >= eeprom.pre_cfg.warning_threshold_sec) // 15 sec
                    {
                        output_array[array_idx] = 1;
                        led_row = (row_value != 0) ? row_value - 1 : 3;
                        LED28_Control(switches[i].day, led_row + MORNING, 1);
                        
                        ESP_LOGI("TAG", "ROW: %d, COLUMN: %s", row_value, switches[i].day == MON ? "MON" : switches[i].day == TUE ? "TUE" :switches[i].day == WED ? "WED" : switches[i].day == THU ? "THU" :switches[i].day == FRI ? "FRI" : switches[i].day == SAT ? "SAT" : "SUN");
                         
                    }
                }
            }
        } 
        else 
        {
            // Reset state and timer when switch released
            if(temp_precfg.warn_th[array_idx].last_time != -1) 
            {
                state.columns[i] = 0;
                temp_precfg.warn_th[array_idx].last_time = -1;
                temp_precfg.warn_th[array_idx].start_time = 0;
            }
            
            if(output_array[array_idx] == 1) 
            {
                state.columns[i] = 0;
                // Reset output and LED when switch was previously active
                output_array[array_idx] = 0;
                led_row = (row_value != 0) ? row_value - 1 : 3;
                LED28_Control(switches[i].day, led_row + MORNING, 0);
            }
            
            if(prolonged_sensed == 1)
            {
                ESP_LOGI("PROLONG", "Prolonged epoch: %lu", eeprom.prolong_action[array_idx].prolonged_epoch[(int)(eeprom.prolong_action[array_idx].prolong_cnt)]);
                // Reset state when switch released
                temp_precfg.warn_th[array_idx].last_time = -1;
                prolonged_sensed = 0;
                write_eeprom((char *)&eeprom);
            }
        }
        // else if(output_array[array_idx] == 1) 
        // {
        //     // Reset state when switch released
        //     output_array[array_idx] = 0;
        //     state.columns[i] = 0;
        //     led_row = (row_value != 0) ? row_value - 1 : 3;
        //     LED28_Control(switches[i].day, led_row + MORNING, 0);
        //     temp_precfg.warn_th[array_idx].last_time = -1;
        // }
    }

    // Handle special case for PB29 model bottle switch
    if(strstr(model_info, "PB29") && row_value == 3) 
    {
        gpio_hold_dis(BOTTLE_SWITCH);
        
        bool bottle_active = (eeprom.Device_MGNT_Polar == CABINET_POLAR) ?
            gpio_get_level(BOTTLE_SWITCH) == 1 :
            gpio_get_level(BOTTLE_SWITCH) == 0;

        if(bottle_active)
        {
            if(++state.bottle_cnt >= limit) {
                sensed = 1;
                state.bottle_cnt = 0;
                output_array[28] = 1;
                RGB_Buzzer_Control(-1, -1, 1, -1);
            }
        } 
        else if(output_array[output_array_mapping(row_value, 7)] == 1) 
        {
            output_array[28] = 0;
            RGB_Buzzer_Control(-1, -1, 0, -1);
            state.bottle_cnt = 0;
        }   
        gpio_hold_en(BOTTLE_SWITCH);
    }
    return sensed;
}

// char column_handle(int row_value)
// {
//     char sensed=0;
//     char static column_0, column_1, column_2, column_3, column_4, column_5, column_6, last_row, bottle_cnt;
//     volatile _precfg_threshold temp_precfg; 
//     if(row_value<0 || row_value>3)return(-1);
//     if(last_row != row_value){last_row = row_value;column_0 = 0; column_1 = 0; column_2 = 0; column_3 = 0; column_4 = 0; column_5 = 0; column_6 = 0;}
//     //ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "SUN");
//     //vTaskDelay(1/portTICK_PERIOD_MS);
//     esp_rom_delay_us(50);
//     char temp=0;
//     if(eeprom.Device_MGNT_Polar == CABINET_POLAR)
//     {
//         ESP_LOGI("TAG", "ROWWWWWWWWW : %d", row_value);
//         if(gpio_get_level(SWITCH_1) == 1) 
//         {
            
//             column_0++;
//             if(column_0 >= limit)
//             {
//                 sensed=1;
//                 column_0 = 0;
//                 if(temp_precfg.warn_th[output_array_mapping(row_value,1)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,1)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,1)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,1)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(SUN,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "SUN");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,1)]==1)
//         {  //  do we need to save this event or not
//             output_array[output_array_mapping(row_value,1)] = 0;
//             column_0 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(SUN,temp+MORNING,0);
//         }

//         if(gpio_get_level(SWITCH_2) == 1) 
//         {
//             column_1++;
//             if(column_1 >= limit)
//             {
//                 sensed=1;
//                 column_1 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,2)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,2)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,2)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,2)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(SAT,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "SAT");
//                     }
//                 }

//             }
//         }
//         else if (output_array[output_array_mapping(row_value,2)]==1)
//         {
//             output_array[output_array_mapping(row_value,2)] = 0;
//             column_1 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(SAT,temp+MORNING,0);
//         }

//         if(gpio_get_level(SWITCH_3) == 1) 
//         {
//             column_2++;
//             if(column_2 >= limit)
//             {
//                 sensed=1;
//                 column_2 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,3)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,3)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,3)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,3)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(FRI,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "FRI");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,3)]==1)
//         {
//             output_array[output_array_mapping(row_value,3)] = 0;
//             column_2 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(FRI,temp+MORNING,0);
//         }

//         if(gpio_get_level(SWITCH_4) == 1) 
//         {
//             column_3++;
//             if(column_3 >= limit)
//             {
//                 sensed=1;
//                 column_3 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,4)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,4)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,4)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,4)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(THU,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "THU");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,4)]==1)
//         {
//             output_array[output_array_mapping(row_value,4)] = 0;
//             column_3 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(THU,temp+MORNING,0);
//         }

//         if(gpio_get_level(SWITCH_5) == 1) 
//         {
//             column_4++;
//             if(column_4 >= limit)
//             {
//                 sensed=1;
//                 column_4 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,5)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,5)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,5)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,5)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(WED,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "WED");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,5)]==1)
//         {
//            output_array[output_array_mapping(row_value,5)] = 0;
//             column_4 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(WED,temp+MORNING,0);
//         }

//         if(gpio_get_level(SWITCH_6) == 1) 
//         {
//             column_5++;
//             if(column_5 >= limit)
//             {
//                 sensed=1;
//                 column_5 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,6)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,6)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,6)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,6)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(TUE,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "TUE");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,6)]==1)
//         {
//             output_array[output_array_mapping(row_value,6)] = 0;
//             column_5 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(TUE,temp+MORNING,0);
//         }

//         if(gpio_get_level(SWITCH_7) == 1) 
//         {
//             column_6++;
//             if(column_6 >= limit)
//             {
//                 sensed=1;
//                 column_6 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,7)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,7)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,7)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,7)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(MON,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "MON");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,7)]==1)
//         {
//             output_array[output_array_mapping(row_value,7)] = 0;
//             column_6 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(MON,temp+MORNING,0);
//         }
        
//         //  Here We will scan the 29th Timer id
        
//         if((strstr(model_info,"PB29")) && (row_value == 3))
//         {
//             gpio_hold_dis(BOTTLE_SWITCH);
//             if(gpio_get_level(BOTTLE_SWITCH) == 1) 
//             {
//                 bottle_cnt++;
//                 if(bottle_cnt >= limit)
//                 {
//                     sensed=1;
//                     bottle_cnt = 0;
//                     output_array[28] = 1;
//                     RGB_Buzzer_Control(-1,-1,1,-1);
//                    // ESP_LOGI("TAG", "BOTTLE SWITCH ENABLE");
//                 }
//             }
//             else if (output_array[output_array_mapping(row_value,7)]==1)
//             {
//                 output_array[28] = 0;
//                 RGB_Buzzer_Control(-1,-1,0,-1);
//                 bottle_cnt = 0;
//             }
//             gpio_hold_en(BOTTLE_SWITCH);
//         }
//     }
//     else if(eeprom.Device_MGNT_Polar == PCBA_POLAR)
//     {
//         // ESP_LOGI("TAG", "ROWWWWWWWWW11111111111 : %d", row_value);
//         if(gpio_get_level(SWITCH_1) == 0) 
//         {
            
//             column_0++;
//             if(column_0 >= limit)
//             {
//                 sensed=1;
//                 column_0 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,1)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,1)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,1)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,1)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(SUN,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "SUN");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,1)]==1)
//         {
//             output_array[output_array_mapping(row_value,1)] = 0;
//             column_0 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(SUN,temp+MORNING,0);
//         }
//         if(gpio_get_level(SWITCH_2) == 0) 
//         {
//             column_1++;
//             if(column_1 >= limit)
//             {
//                 sensed=1;
//                 column_1 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,2)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,2)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,2)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,2)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(SAT,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "SAT");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,2)]==1)
//         {
//             output_array[output_array_mapping(row_value,2)] = 0;
//             column_1 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(SAT,temp+MORNING,0);
//         }
//         if(gpio_get_level(SWITCH_3) == 0) 
//         {
//             column_2++;
//             if(column_2 >= limit)
//             {
//                 sensed=1;
//                 column_2 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,3)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,3)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,3)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,3)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(FRI,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "FRI");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,3)]==1)
//         {
//             output_array[output_array_mapping(row_value,3)] = 0;
//             column_2 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(FRI,temp+MORNING,0);
//         }
//         if(gpio_get_level(SWITCH_4) == 0) 
//         {
//             column_3++;
//             if(column_3 >= limit)
//             {
//                 sensed=1;
//                 column_3 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,4)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,4)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,4)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,4)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(THU,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "THU");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,4)]==1)
//         {
//             output_array[output_array_mapping(row_value,4)] = 0;
//             column_3 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(THU,temp+MORNING,0);
//         }
//         if(gpio_get_level(SWITCH_5) == 0) 
//         {
//             column_4++;
//             if(column_4 >= limit)
//             {
//                 sensed=1;
//                 column_4 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,5)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,5)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,5)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,5)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(WED,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "WED");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,5)]==1)
//         {
//            output_array[output_array_mapping(row_value,5)] = 0;
//             column_4 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(WED,temp+MORNING,0);
//         }
//         if(gpio_get_level(SWITCH_6) == 0) 
//         {
//             column_5++;
//             if(column_5 >= limit)
//             {
//                 sensed=1;
//                 column_5 = 0;

//                 if(temp_precfg.warn_th[output_array_mapping(row_value,6)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,6)].last_time = (esp_log_timestamp());
//                 }
//                 else
//                 {
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,6)].last_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         output_array[output_array_mapping(row_value,6)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(TUE,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "TUE");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,6)]==1)
//         {
//             output_array[output_array_mapping(row_value,6)] = 0;
//             column_5 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(TUE,temp+MORNING,0);
//         }
//         if(gpio_get_level(SWITCH_7) == 0) 
//         {
//             column_6++;
//             if(column_6 >= limit)
//             {
//                 sensed=1;
//                 column_6 = 0;
                

//                 ESP_LOGI("tag", " diff <<<<%ld>%ld>>",((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,7)].start_time), eeprom.pre_cfg.warning_threshold_sec);
//                 if(temp_precfg.warn_th[output_array_mapping(row_value,7)].last_time == 0)
//                 {
//                     temp_precfg.warn_th[output_array_mapping(row_value,7)].last_time = 1;
//                     temp_precfg.warn_th[output_array_mapping(row_value,7)].start_time = (esp_log_timestamp());
//                     ESP_LOGI("time", "warn_th  %ld ",temp_precfg.warn_th[output_array_mapping(row_value,7)].start_time);
//                 }
//                 else
//                 {
//                     ESP_LOGI("TAG", "TIME  %ld, %ld ",(esp_log_timestamp()), temp_precfg.warn_th[output_array_mapping(row_value,7)].start_time);
//                     if(((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,7)].start_time) >= eeprom.pre_cfg.warning_threshold_sec )
//                     {
//                         ESP_LOGI("tag22222", " diff <<<<%ld>>>",((esp_log_timestamp()) - temp_precfg.warn_th[output_array_mapping(row_value,7)].start_time));
//                         temp_precfg.warn_th[output_array_mapping(row_value,7)].last_time = 0;
//                         output_array[output_array_mapping(row_value,7)] = 1;
//                         if(row_value!=0){temp=row_value-1;}
//                         else{
//                             temp=3;
//                         }
//                         LED28_Control(MON,temp+MORNING,1);                
//                         ESP_LOGI("TAG", "ROW : %d, COLUMN: %s", row_value, "MON");
//                     }
//                 }
//             }
//         }
//         else if (output_array[output_array_mapping(row_value,7)]==1)
//         {
//             output_array[output_array_mapping(row_value,7)] = 0;
//             column_6 = 0;
//             if(row_value!=0){temp=row_value-1;}
//                 else{
//                     temp=3;
//                 }
//                 LED28_Control(MON,temp+MORNING,0);
//         }
//         if((strstr(model_info,"PB29")) && (row_value == 3))
//         {
//             gpio_hold_dis(BOTTLE_SWITCH);
//             if(gpio_get_level(BOTTLE_SWITCH) == 0) 
//             {
//                 bottle_cnt++;
//                 if(bottle_cnt >= limit)
//                 {
//                     sensed=1;
//                     bottle_cnt = 0;
//                     output_array[28] = 1;
//                     RGB_Buzzer_Control(-1,-1,1,-1);
//                    // ESP_LOGI("TAG", "BOTTLE SWITCH ENABLE");
//                 }
//             }
//             else if (output_array[output_array_mapping(row_value,7)]==1)
//             {
//                 output_array[28] = 0;
//                 RGB_Buzzer_Control(-1,-1,0,-1);
//                 bottle_cnt = 0;
//             }
//             gpio_hold_en(BOTTLE_SWITCH);
//         }
//     }
    
//     return(sensed);
// }

int ret_row = -1;
char Keypad_Scan(char mode)
{
    char value=0;
    for(int i=0;i<4;i++)
    {
        //ESP_LOGI("111111","");
        ret_row = row_handle();
        value = column_handle(ret_row);  
        if(value!=0)break; 
    } 
    ROW1_HIGH;   ROW2_HIGH;  ROW3_HIGH;  ROW4_HIGH;
    if(value==0)memset(output_array_pre,0x00,sizeof(output_array_pre));
    if((value != 0) && (mode != 0))
    {
        f.g_10sec_F = 1;// to immediately alert the user about correct compartment
        for(int i = 0; i < MAXIMUM_SUPPORTED_BOXES; i++)
        {
            if(output_array[i] == 1)
            {
                if(output_array_pre[i] == 0)
                {
                    uint32_t current_epoch=0;
                    output_array_pre[i]=1;
                     
                    char ret = READ_RTC_and_GET_Epoch(&current_epoch);
                    if(ret == 'V')
                    {                   
                        int res = validate_occureed_event(i,current_epoch);
                        if(res == 1)
                        {
                            convertIdToRowColumn_GlowLedMatrix(i,3);
                            ESP_LOGI("KYSCN", "TRUE : EVENT Occured");                            
                            switch_off_audio();
                            RGB_Buzzer_Control(0,0,0,0);                            
                            Stop_Buzzer();
                        }
                        else if(res == 2)
                        {
                            convertIdToRowColumn_GlowLedMatrix(i,3);
                            ESP_LOGI("KYSCN", "TRUE but false : EVENT Occured");
                            
                            switch_off_audio();
                            RGB_Buzzer_Control(0,0,0,0);
                            Stop_Buzzer();
                        }
                        else if(res == 0)
                        {                            
                            ESP_LOGI("KYSCN", "FALSE : EVENT Occured");
                            RGB_Buzzer_Control(0,0,0,0);
                            Blow_Buzzer(10,1000);
                        }
                    }
                    
                }
            }
            else
            {
                output_array_pre[i]=0;
            }
        }         
    }
    return(value);
}

int validate_occureed_event(uint8_t box_id,uint32_t current_epoch)
{
    char flag = -1;
    	for(int i=0;i<eeprom.total_timers_configured;i++)
        {
           if(eeprom.config_alarm_time[i].Box_ID == box_id)
           {   
            flag = 0;          
                if(eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS == 1)
                {
                    if(((eeprom.config_alarm_time[i].timer_epoch+eeprom.config_alarm_time[i].total_alarm_duratition_epoch) > (current_epoch)) &&(eeprom.config_alarm_time[i].timer_epoch <= (current_epoch)))   
				    {
                        ESP_LOGI("MmMmMmMMMMMMMMMMMMMMMMMMMMM", "TRUE : EVENT Occured");
                        eeprom.config_alarm_time[i].timer_result_epoch  = current_epoch;	
                        eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS  = 2;	
                        write_eeprom((char *)&eeprom);
                        return (1);
                    }
                }
                else if(eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS == 3)
                {
                    eeprom.config_alarm_time[i].timer_result_epoch  = current_epoch;	
                    eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS  = 4;					
                    write_eeprom((char *)&eeprom);
                    return(2);
                }
                else if((eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS == 0)  || (eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS == 2) || (eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS == 4))
                {
                    eeprom.wrong_action[i].box_cnt++;
                            if(eeprom.wrong_action[i].box_cnt>120)
                            {
                                eeprom.wrong_action[i].box_cnt = 120;
                            }
                            if(eeprom.wrong_action[i].box_cnt<10)
                            {
                                eeprom.wrong_action[i].epoch[(int)(eeprom.wrong_action[i].box_cnt)] = current_epoch;
                            }
                            write_eeprom((char *)&eeprom);
                        return (0);
                }
           } 
        }
       if(flag != 0)return(0); 
    return(-1);
}

uint8_t read_gpio_status(uint8_t gpio_num)
{
    if(gpio_get_level((gpio_num_t)gpio_num))
    {        
        return (1);
    }
    else
    {
        return (0);
    }
   
}

char sense_button_status_polling(char threshold)
{
    uint16_t PWR_OFF_ON_SWITCH_cnt = 0, MODE_SWITCH_cnt=0;
    for(int i = 0; i <300;i++)
    {
        if(read_gpio_status(PWR_OFF_ON_SWITCH) == 0)
        {
            ESP_LOGI("CCCCCCCCCCCCC","");
            PWR_OFF_ON_SWITCH_cnt++;
            if(PWR_OFF_ON_SWITCH_cnt >= threshold) 
            {
                PWR_OFF_ON_SWITCH_cnt = 0;  
                if(eeprom.PWR_ON_OFF==0)              
                {
                    
                    eeprom.PWR_ON_OFF = 1; 
                    write_eeprom((char *)&eeprom); 
                    while(read_gpio_status(PWR_OFF_ON_SWITCH)==0){vTaskDelay(10/portTICK_PERIOD_MS);}
                    return(2); 
                }
                return(1);
                //break;              
            }
        }
        else
        {
            PWR_OFF_ON_SWITCH_cnt = 0;
        }
        if(eeprom.PWR_ON_OFF!=0)  
        {
            if((read_gpio_status(MODE_SWITCH)==0) || (read_gpio_status(ALARM_SWITCH)==0))
            {
                MODE_SWITCH_cnt++;
                if(MODE_SWITCH_cnt >= threshold) 
                {
                    MODE_SWITCH_cnt = 0;  
                    // if((read_gpio_status(MODE_SWITCH)==0))
                    // {
                    //     if(f.g_panic_alert_F == 0)
                    //     {f.g_panic_alert_F = 1;}
                    //     else{f.g_panic_alert_F=0;}
                    //     f.g_10sec_F = 1;
                    // }
                    return(1); 
                    //break;              
                }
            }
            else
            {
                MODE_SWITCH_cnt=0;
            }
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    return(0);
}
char sense_button(char mode)
{
    char flag = 0;
    static uint16_t alarm_snooze_cnt=0,MODE_cnt=0,PWR_OFF_ON_SWITCH_cnt=0;
    char button_threshold = 40;
    if(mode == 0)
    {
        button_threshold  = 1;
    }
    if(f.g_backlight_Off_F == 1)
    {
        if((read_gpio_status(ALARM_SWITCH) == 0) ||(read_gpio_status(PWR_OFF_ON_SWITCH)==0) || (read_gpio_status(MODE_SWITCH)==0))
        { 
            f.g_reset_counter_and_flag_F = 1;
        }
        return (0);
    }
    if(read_gpio_status(ALARM_SWITCH) == 0)
    {        
        alarm_snooze_cnt++;
        if(alarm_snooze_cnt >= button_threshold) 
        {
            f.g_reset_counter_and_flag_F = 1;
            alarm_snooze_cnt = 0;
            flag = 1;
            g_Buzzer_Threshold = 0;
            g_Buzzer_toggle_interval = 0;
            switch_off_audio();
            RGB_Buzzer_Control(0,0,0,0);
            ESP_LOGI("SENSE BUTTON", "SNOOZE BUTTON ON");
            LED_MATRIX_RESET();
            Buzzer_Pattern(200);
            while(read_gpio_status(ALARM_SWITCH)==0){vTaskDelay(10/portTICK_PERIOD_MS);}
            // f.g_panic_alert_F = 0;
        }
        
    }
    else
    {
        alarm_snooze_cnt = 0;
    }

    if(f.g_panic_alert_F == 1)
    {
        button_threshold = 10;
    }
    else
    {
        button_threshold = 80;
    }

    if(read_gpio_status(MODE_SWITCH)==0)
    {
        MODE_cnt++;
        ESP_LOGI("SENSE BUTTON", "===%d===%d==",MODE_cnt, button_threshold);
        if(MODE_cnt >= button_threshold) 
        {
            f.g_reset_counter_and_flag_F = 1;
            MODE_cnt = 0;
            flag = 1;
            if(f.g_panic_alert_F == 0)
            {f.g_panic_alert_F = 1;ESP_LOGI("PANIC BUTTON", "PANIC_SWITCH ON");}
            else{f.g_panic_alert_F=0;ESP_LOGI("PANIC BUTTON", "PANIC_SWITCH OFF");}
            f.g_10sec_F = 1;
            while(read_gpio_status(MODE_SWITCH)==0){vTaskDelay(10/portTICK_PERIOD_MS);}
        }
        
    }
    else
    {
        MODE_cnt = 0;
        // ESP_LOGI("SENSE BUTTON", "MODE_SWITCH OFF");
    }
    if(read_gpio_status(PWR_OFF_ON_SWITCH)==0)
    {
        PWR_OFF_ON_SWITCH_cnt++;
        if(f.g_backlight_Off_F==1)
        {
            f.g_reset_counter_and_flag_F = 1;
        }
        if(PWR_OFF_ON_SWITCH_cnt >= 80) 
        {
            PWR_OFF_ON_SWITCH_cnt = 0;
            ESP_LOGI("SENSE BUTTON", "PWR_OFF_ON_SWITCH ON");
            Buzzer_Pattern(200);
            if(eeprom.PWR_ON_OFF == 1) 
            {
                eeprom.PWR_ON_OFF = 0;
                write_eeprom((char *)&eeprom);
                for(int i=0;i<5;i++)
                {
                    LED_MATRIX_SET();vTaskDelay(100/portTICK_PERIOD_MS);
                    LED_MATRIX_RESET();vTaskDelay(100/portTICK_PERIOD_MS);
                }
                while(read_gpio_status(PWR_OFF_ON_SWITCH)==0){vTaskDelay(10/portTICK_PERIOD_MS);}
                PB_esp_restart();
            }
            // else 
            // {
            //     eeprom.PWR_ON_OFF = 1; 
            //     write_eeprom((char *)&eeprom);
            //     flag = 1;
            //     // PB_esp_restart();
            // }
        }
        // enter_into_sleep_mode(20);
    }
    else if((PWR_OFF_ON_SWITCH_cnt >= button_threshold)) 
    {
        PWR_OFF_ON_SWITCH_cnt = 0;
        flag = 1;
        sleep_handle(0);
    }
    else 
    {
        PWR_OFF_ON_SWITCH_cnt = 0;
        // ESP_LOGI("SENSE BUTTON", "PWR_OFF_ON_SWITCH OFF");
    }
    
    return flag;
}

