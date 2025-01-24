#pragma once

#define EXAMPLE_STD_BCLK_IO1        AUDIO_BCLK        // I2S bit clock io number   I2S_BCLK
#define EXAMPLE_STD_WS_IO1          AUDIO_LRCLK       // I2S word select io number    I2S_LRC
#define EXAMPLE_STD_DOUT_IO1        AUDIO_IN          // I2S data out io number    I2S_DOUT
#define EXAMPLE_STD_DIN_IO1         GPIO_NUM_NC       // I2S data in io number

#define EXAMPLE_BUFF_SIZE               2048
#define SAMPLE_RATE 8000



void Sound_play_task();
void sound_task_enable(void);
void Start_play_sound();
static void i2s_example_init_std_simplex(void);
void Read_sound_first_packet_info(FileInfo *info);
void switch_off_audio(void);
