#include "header.h"

static i2s_chan_handle_t tx_chan;        // I2S tx channel handler

int g_sound_index_value = -1;



void Start_play_sound()
{
    // g_sound_index_value = 1;
    g_sound_index_value = 5;
}
/// @brief 
/// @param info 
/// @return 0
void Read_sound_first_packet_info(FileInfo *info)
{
    char temp[20] = {0}; char sound_data[500] = {0};
    
    // Decode_Filename(g_sound_index_value,(char*)info->file_name);
    Decode_Filename(1,(char*)info->file_name);
    ESP_LOGI("File Name ","%s",(char*)info->file_name);
    ReadDataFromFS((char*)info->file_name,0,(char*)&sound_data, 496);
    // total File size
    memset((char*)&temp,0x00,sizeof(temp));
    get_parameter_from_data((char*)&temp,(char*)sound_data,",",2,"FTP",0,0,8);
    info->file_size = atoi(temp);       memset((char*)&temp, 0x00, sizeof(temp));
    // Total number No OF packet in the file
    get_parameter_from_data((char*)&temp,(char*)sound_data,",",3,"FTP",0,0,4);
    info->files_total_packet = atoi(temp); memset((char*)&temp, 0x00, sizeof(temp));
    // File Checksum
    get_parameter_from_data((char*)&temp,(char*)sound_data,",",4,"FTP",0,0,4);
    info->file_checksum = atoi(temp);   memset((char*)&temp, 0x00, sizeof(temp));
    // File Check xor
    get_parameter_from_data((char*)&temp,(char*)sound_data,",",5,"FTP",0,0,4);
    info->file_checkxor = atoi(temp);   memset((char*)&temp, 0x00, sizeof(temp));
    // Chunk size
    get_parameter_from_data((char*)&temp,(char*)sound_data,",",6,"FTP",0,0,4);
    info->packet_data_size = atoi(temp);
    ESP_LOGI("file","chksum %x, chkxor %x, %ld",info->file_checksum, info->file_checkxor, info->file_size);

}

uint8_t sound_buffer[EXAMPLE_BUFF_SIZE] = {0},channel_enabled=0;
uint8_t sound_buffer_stereo[EXAMPLE_BUFF_SIZE*2] = {0};
void Sound_play_task()
{
    FileInfo sound_info;
    size_t w_bytes = 0;
    uint32_t offset = (496+44);
   // unsigned char volume=2;// lower means high volume
  // g_sound_index_value=1;
    while (1)
    {
       //g_sound_index_value=1;
        if(g_sound_index_value != -1)
        {
            if(channel_enabled==0)
            {
            ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
            channel_enabled=1;
            }
            
            f.g_audio_ON = 1; offset = 496 + 44;
            vTaskDelay(100/portTICK_PERIOD_MS);
            Read_sound_first_packet_info((FileInfo *)&sound_info);
            while (1)
            {
                if(g_sound_index_value == -1)
                {
                    break;
                }

                if ( offset > sound_info.file_size)
                {
                    offset = (496+44);
                    ESP_LOGI("SOUND","...%d",g_sound_index_value);
                    g_sound_index_value--;
                    ESP_LOGI("SOUND","...%d",g_sound_index_value);
                    vTaskDelay(500/portTICK_PERIOD_MS);
                    // break;
                }

                

                ReadDataFromFS((char*)&sound_info.file_name[0],offset,(char*)&sound_buffer[0],EXAMPLE_BUFF_SIZE);
                for (int j = 0,i=0; j < EXAMPLE_BUFF_SIZE; ) 
                { 
                    // sound_buffer_stereo[i+1] = sound_buffer[j]>>2;  
                    // sound_buffer_stereo[i] = (sound_buffer[j]&0x03)<<6; 
                    // sound_buffer_stereo[i+3] = sound_buffer[j]>>2; 
                    // sound_buffer_stereo[i+2] = (sound_buffer[j]&0x03)<<6; 

                    sound_buffer_stereo[i]   = sound_buffer[j];  
                    sound_buffer_stereo[i+1] = sound_buffer[j+1]; 
                    sound_buffer_stereo[i+2] = sound_buffer[j]; 
                    sound_buffer_stereo[i+3] = sound_buffer[j+1]; 
                    
                    // printf("%x,%x,%x,%x\n", buffer[i],sample_value,train_annoucement[offset],train_annoucement[offset+1]);
                    j+=2;
                    i+=4;
                }
                offset += (EXAMPLE_BUFF_SIZE);

                if (i2s_channel_write(tx_chan,(char*)&sound_buffer_stereo[0], EXAMPLE_BUFF_SIZE*2, &w_bytes, 1000) == ESP_OK)
                {
                    // printf("Write Task: i2s write %d bytes\n", w_bytes);
                } 
                else 
                {
                    printf("Write Task: i2s write failed\n");
                }
                //printf("size %ld\noffset %lu\n", sound_info.file_size,offset);
                 vTaskDelay(10/portTICK_PERIOD_MS);
            }
            switch_off_audio();
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    
    ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));
    vTaskDelete(NULL);
}

void switch_off_audio(void)
{
    if(channel_enabled == 1)
    {
        channel_enabled = 0;
        ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));
    }
    f.g_audio_ON = 0;
    g_sound_index_value = -1;
}

static void i2s_example_init_std_simplex(void) 
{
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, &tx_chan, NULL));
    i2s_std_config_t tx_std_cfg = {
            .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
            .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT,I2S_SLOT_MODE_STEREO),

            .gpio_cfg = {
                .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
                .bclk = EXAMPLE_STD_BCLK_IO1,
                .ws   = EXAMPLE_STD_WS_IO1,
                .dout = EXAMPLE_STD_DOUT_IO1,
                .din  = EXAMPLE_STD_DIN_IO1,
                .invert_flags = {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv   = false,
                },
            },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &tx_std_cfg));
}

void sound_task_enable(void)
{
    g_sound_index_value =  -1;
    i2s_example_init_std_simplex();
    channel_enabled=0;
    //ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
    /* Step 4: Create writing and reading task */
    xTaskCreate(Sound_play_task, "Sound_play_task", 4096, NULL, 5, NULL);
}