#ifndef GLOBAL_C_
#define GLOBAL_C_

#include "header.h"
char model_info[10] = {0};
int g_led_toggle_cnt,g_led_toggle_state,g_led_toggle_threshold;
char INIT_FR  = 0xA5, g_Buzzer_toggle_interval;
volatile int g_Buzzer_Threshold;
volatile _flags f;
volatile _eeprom eeprom;
volatile _MEMORY_PARAM g_memory_parameter;
// volatile PB_COMPRASION_PARAM  g_comprasion_param;
const char monthDays[]  = {31,28,31,30,31,30,31,31,30,31,30,31};
/**
 * The function `get_parameter_from_data` extracts a parameter from a given data string based on
 * specified delimiters and conditions, storing the result in an output buffer.
 * 
 * @param out The `out` parameter is a pointer to a character array where the extracted parameter will
 * be stored by the function `get_parameter_from_data`.
 * @param data The function `get_parameter_from_data` takes several parameters and extracts a specific
 * parameter from a given data string based on delimiters and other conditions. Here's a breakdown of
 * the parameters:
 * @param delimiter The `delimiter` parameter is a character pointer that specifies the delimiter used
 * to separate different parts of the `data` string. In the provided function
 * `get_parameter_from_data`, the delimiter is used to extract a specific parameter from the `data`
 * string based on its position.
 * @param number The `number` parameter in the `get_parameter_from_data` function is used to specify
 * which occurrence of the delimiter-separated substring to extract from the input `data`. It indicates
 * the position of the desired substring within the data.
 * @param const_strng The `const_strng` parameter in the provided function `get_parameter_from_data` is
 * a pointer to a constant string that is used as a reference point for searching within the `data`
 * string. If the `const_strng` is found within the `data` string, the search for the
 * @param termination The `termination` parameter in the `get_parameter_from_data` function is used to
 * specify a string that marks the end of the data segment to be processed. If this string is found in
 * the data, the processing stops at that point. If the `termination` parameter is not provided or is
 * set
 * @param exact_len The `exact_len` parameter in the `get_parameter_from_data` function is used to
 * specify the exact length that the output parameter `out` should have. If the length of the output
 * parameter `out` is not equal to the value specified in `exact_len`, the function will return 0
 * @param max_len The `max_len` parameter in the `get_parameter_from_data` function is used to specify
 * the maximum length of the output string `out`. If `max_len` is not zero, the function will copy at
 * most `max_len` characters from the extracted parameter into the `out` buffer.
 * 
 * @return The function `get_parameter_from_data` returns either 0 or 1. It returns 0 in case of an
 * error or failure, and it returns 1 if the function successfully retrieves and processes the
 * parameter from the provided data.
 */

char get_parameter_from_data(char *out, char *data, char *delimiter, char number, char *const_strng, char *termination, int exact_len, int max_len)
{
    unsigned char MAX_SUB_STRG_LEN = 160;
    char tttb[162] = {0};
    char *l_ptr, *ptr, *str_ptr, *stp_ptr;
    ptr = data;
    str_ptr = 0;
    stp_ptr = 0;

    if (const_strng != 0)
    {
        l_ptr = strstr(ptr, const_strng);
        if (l_ptr == 0)
        {
            return (0);
        }
        str_ptr = l_ptr;
        ptr= str_ptr;             // we will search after the constant string kkkkkkkkk
    }

    if (termination != 0)
    {
        l_ptr = strstr(ptr, termination);
        if (l_ptr == 0)
        {
            return (0);
        }
        else if (*(l_ptr + 1) != 0)
        {
            return (0);
        }
        else
        {
            *l_ptr = 0;
            stp_ptr = l_ptr;
        }
    }

    l_ptr = strstr(ptr, delimiter);
    if ((l_ptr != 0) && ((str_ptr == 0) || (l_ptr >= str_ptr)) && ((stp_ptr == 0) || (l_ptr < stp_ptr)))
    {
        int i;
        for (i = 0; i < number; i++)
        {
            ptr = l_ptr + strlen(delimiter);
            l_ptr = strstr(ptr, delimiter);
            if (l_ptr == '\0')
                break;
        }
        if ((l_ptr == '\0'))
        {
            if (i == (number - 1))
            {
                int k;
                for (k = 0; k < MAX_SUB_STRG_LEN && *ptr != 0; k++)
                {
                    tttb[k] = *ptr++;
                }
                tttb[k] = 0;
            }
            else
            {
                // printf("%s_%s4",ptr, __func__);
                return (0);
            }
        }
        else
        {
            char *ter;
            ter = strstr(ptr, delimiter);
            if ((ter - ptr) <= MAX_SUB_STRG_LEN)
            {
                strncpy(tttb, ptr, (ter - ptr));
            }
            else
            {
                strncpy(tttb, ptr, MAX_SUB_STRG_LEN);
            }
        }
    }
    else if (number == 0)
    {
        if ((strlen(data)) <= MAX_SUB_STRG_LEN)
        {
            strcpy(tttb, data);
        }
        else
        {
            strncpy(tttb, data, MAX_SUB_STRG_LEN);
        }
    }
    else
    {
        return (0);
    }

    if (max_len != 0)
    {
        strncpy(out, tttb, max_len);
    }
    else
    {
        strcpy(out, tttb);
    }

    if (exact_len != 0)
    {
        if (strlen(out) != exact_len)
        {
            return (0);
        }
    }

    return (1);
}

/**
 * The function `convert_epoch_into_gmttime_and_date` converts a given epoch time into GMT date and
 * time components.
 * 
 * @param epoch The `epoch` parameter in the function `convert_epoch_into_gmttime_and_date` is a time_t
 * variable representing the number of seconds that have passed since the Unix epoch (00:00:00 UTC on 1
 * January 1970).
 * @param date The function `convert_epoch_into_gmttime_and_date` takes a Unix epoch time and converts
 * it into GMT time and date. The `date` parameter is a pointer to a character array where the function
 * will store the calculated date in the format "DDMMYYYY".
 * @param time The `convert_epoch_into_gmttime_and_date` function takes an epoch time in seconds and
 * converts it into GMT time and date. The function then stores the date in the `date` parameter and
 * the time in the `time` parameter.
 */
void convert_epoch_into_gmttime_and_date(time_t epoch,char* date,char* time)
{
    short int year = 1970;
    unsigned char month;
    short int monthLength;
    int days = 0;
    char l_temp_store[20];
    short int yr=0,mn=0,dy = 0,hh=0,mm=0,sec=0;

    sec=epoch%60;
    epoch/=60; // now it is minutes
    mm=epoch%60;
    epoch/=60; // now it is hours
    hh=epoch%24;
    epoch/=24; // now it is days
    
    
    while((days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch)
    {
        year++;
    }
    yr=year-1900;
    
    days -= LEAP_YEAR(year) ? 366 : 365;
    epoch -= days; // now it is days in this year, starting at 0
    
    
    days=0;
    month=0;
    monthLength=0;
    for (month=0; month<12; month++)
    {
        if (month==1)
        {
            // february
            if (LEAP_YEAR(year))
            {
                monthLength=29;
            }
            else
            {
                monthLength=28;
            }
        }
        else
        {
            
            monthLength = monthDays[month];
            
        }
        
        if (epoch>=monthLength)
        {
            epoch-=monthLength;
        }
        else
        {
            break;
        }
    }
    mn=month;
    dy=epoch+1;
    
    
    
    
    /*	Store calculated date to gps structure */
    itoa(dy,l_temp_store,10);
    l_temp_store[2] = '\0';
    
    if(dy <10)
    {
        strcpy(date,("0"));
        strcat(date,l_temp_store);
    }
    else
    {
        strcpy(date,l_temp_store);
    }


    mn += 1;
    itoa(mn,l_temp_store,10);
    l_temp_store[2] = '\0';

    
    if(mn <10)
    {
        strcat(date,("0"));
        strcat(date,l_temp_store);
    }
    else
    {
        strcat(date,l_temp_store);
    }

    if(yr>99)			//year correction change is added 
    yr -= 2000;
    else
    yr -= 1900;

    yr += 1900;
    itoa(yr,l_temp_store,10);
    l_temp_store[2] = '\0';


    if(yr <10)
    {
        strcat(date,("0"));
        strcat(date,l_temp_store);
    }
    else
    {
        strcat(date,l_temp_store);
    }

    /*	Store calculated time to gps structure */
    itoa(hh,l_temp_store,10);
    l_temp_store[2] = '\0';

    if(hh <10)
    {
        strcpy(time,("0"));
        strcat(time,l_temp_store);
    }
    else
    {
        strcpy(time,l_temp_store);
    }

    itoa(mm,l_temp_store,10);
    l_temp_store[2] = '\0';


    if(mm <10)
    {
        strcat(time,("0"));
        strcat(time,l_temp_store);
    }
    else
    {
        strcat(time,l_temp_store);
    }
    itoa(sec,l_temp_store,10);
    l_temp_store[2] = '\0';


    if(sec <10)
    {
        strcat(time,("0"));
        strcat(time,l_temp_store);
    }
    else
    {
        strcat(time,l_temp_store);
    }
}

uint32_t convert_time_date_into_epoch(char* date, char* time)
{
    struct tm timeinfo = {0};
    
    timeinfo.tm_year = (2000 + ((date[4] - '0') * 10 + (date[5] - '0'))) - 1900;  // Year since 1900
    timeinfo.tm_mon  = ((date[2] - '0') * 10 + (date[3] - '0')) - 1;     // Month (0-11)
    timeinfo.tm_mday = ((date[0] - '0') * 10 + (date[1] - '0'));          // Day of the month (1-31)
    timeinfo.tm_hour = ((time[0] - '0') * 10 + (time[1] - '0'));         // Hour (0-23)
    timeinfo.tm_min  = ((time[2] - '0') * 10 + (time[3] - '0'));        // Minute (0-59)
    timeinfo.tm_sec  = ((time[4] - '0') * 10 + (time[5] - '0'));        // Second (0-59)

    // Convert to epoch time
    uint32_t epochTime = mktime(&timeinfo);
    // ESP_LOGI("EPOCH", "date %s, time %s, %d", date, time, epochTime);
    return epochTime;

}

char ascii2hex(char l_char)
{
    if(l_char<='9')
    {
        l_char = l_char-0x30;								// Substracting 0x30 as it is the hex value of 0
    } 
    else
    {
        l_char=(l_char-'A') + 0x0A; 
    }
    return l_char;
}

char PB_esp_restart()
{
    vTaskDelay(2000/portTICK_PERIOD_MS);
    esp_restart();
}

char PB_immediate_restart()
{
    esp_restart();
}

#endif