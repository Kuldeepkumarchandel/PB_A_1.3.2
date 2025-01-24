#include "header.h"

struct stat st;
esp_vfs_littlefs_conf_t conf = {
    .base_path = "/littlefs",
    .partition_label = "storage",
    .format_if_mount_failed = true,
    .dont_mount = false,
};

#define MEMORY_FILE                 "/littlefs/memory.txt"
#define MEMORY_RD_WR_PTR_FILE       "/littlefs/memory_rd_wr_ptr.txt"
FILE *g = NULL;
char WriteDataToFS(char*file_name, int pos, char*pBuffer, int write_size)
{
    // printf("write FILE: %s, pos=%d, size= %d\n",file_name,pos,write_size);
    GetMemoryMutex();
    
        esp_err_t ret = esp_vfs_littlefs_register(&conf);
        if (ret != ESP_OK)
        {Release_Memory_mutex();
            printf("Failed to get LittleFS partition information (%s)\n", esp_err_to_name(ret));
            return(0);
        }

       
        if (stat(file_name, &st) == 0)     
        {
            g = fopen(file_name, "r+");
        }
        else
        {
            g = fopen(file_name, "w");
        }
    
        if(g == NULL)
        {
            fclose(g);
            esp_vfs_littlefs_unregister(conf.partition_label);
            printf("Failed to open file for writing\n");Release_Memory_mutex();
            return 0;
        }

        int seek_result = fseek(g, pos, SEEK_SET);
        if (seek_result < 0)
        {
            fclose(g);
            esp_vfs_littlefs_unregister(conf.partition_label);
            printf("Failed to seek file for writing\n");Release_Memory_mutex();
            return 0;
        }
        size_t fwrite_status = fwrite((char*)pBuffer, 1, (size_t)write_size, g);
        if(fwrite_status<=0)//if(fwrite_status != 0)
        {
            printf("written_len:%d\n", fwrite_status);
        }
    
        // printf("written_len:%d sucessfully\n", fwrite_status);
        fclose(g);
        esp_vfs_littlefs_unregister(conf.partition_label);
    
    Release_Memory_mutex();
    return(1);
}

char ReadDataFromFS(char* file_name, int pos, char*pBuffer, int read_size)
{
    GetMemoryMutex();
    // printf("Read FILE: %s, pos=%d, size= %d\n",file_name,pos,read_size);
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        esp_vfs_littlefs_unregister(conf.partition_label);Release_Memory_mutex();
        printf("Failed to get LittleFS partition information (%s)\n", esp_err_to_name(ret));return(0);
    }

    FILE *f;
    f = fopen(file_name, "r");
    if (f == NULL)
    {
        fclose(f);
        esp_vfs_littlefs_unregister(conf.partition_label);
        printf("Failed to open file for reading %s\n",file_name);Release_Memory_mutex();
        return 0;
    }
    int seek_result = fseek(f, pos, SEEK_SET);
    if (seek_result < 0)
    {
        fclose(f);
        esp_vfs_littlefs_unregister(conf.partition_label);
        printf("Failed to seek file for reading\n");Release_Memory_mutex();
        return 0;
    }        
    size_t fread_result = fread((char*)pBuffer, 1, (size_t)read_size, f);
    if(fread_result != read_size)
    {
        fclose(f);
        esp_vfs_littlefs_unregister(conf.partition_label);
        printf("Failed to read data from  %s\n", file_name);Release_Memory_mutex();
        return 0;
    }
    fclose(f);
    esp_vfs_littlefs_unregister(conf.partition_label);Release_Memory_mutex();
    return(1);
}

char Delete_File(char* file)
{
    
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        Release_Memory_mutex();
        return(0);
    }
    if(unlink(file) == 0)
    {
        esp_vfs_littlefs_unregister(conf.partition_label);
        printf("Delete file :%s",file);
        Release_Memory_mutex();
        return(1);
    }    
    esp_vfs_littlefs_unregister(conf.partition_label);
        Release_Memory_mutex();
    return(0);
}

char Delete_memory_data()
{
   memset((char*)&eeprom.config_alarm_time[0],0x00,sizeof(_CONFIG_ALARM_TIME));
   memset((char*)&eeprom.wrong_action[0],0x00,sizeof(_wrong_action));
    return(1);
}

char Erase_File_System(void)  
{
    GetMemoryMutex();
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        Release_Memory_mutex();
        return(0);
    }
    if(esp_littlefs_format("storage") == 0)
    {
        esp_vfs_littlefs_unregister(conf.partition_label);
        // return(1);
    }    
    esp_vfs_littlefs_unregister(conf.partition_label);
        Release_Memory_mutex();
    return(0);
}

void Read_memory_RDWRptr(uint32_t *wr_ptr, uint32_t *rd_ptr, uint8_t server)
{
	// char read_buff[9]={0};
	// ReadDataFromFS((char *)MEMORY_RD_WR_PTR_FILE,(uint32_t)(server*8),(char*)read_buff,8);
    // memcpy((char*)wr_ptr,(char*)&read_buff[0],sizeof(uint32_t));
    // memcpy((char*)rd_ptr,(char*)&read_buff[4],sizeof(uint32_t));
}

void write_memory_RDWRptr(uint32_t *wr_ptr, uint32_t *rd_ptr, uint8_t server)
{
    // char write_buff[9]={0};
    // memcpy((char*)&write_buff[0],(char*)wr_ptr,sizeof(uint32_t));
    // memcpy((char*)&write_buff[4],(char*)rd_ptr,sizeof(uint32_t));	
	// WriteDataToFS((char*)MEMORY_RD_WR_PTR_FILE,(uint32_t)(server*8),(char*)&write_buff,8);		// done by kuldeep (char*)&write_buff earlier it was (char*)write_buff
}

char write_eeprom(char* eeprom_data)
{
    GetMemoryMutex();
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        
        Release_Memory_mutex();
        printf("Failed to get LittleFS partition information (%s)\n", esp_err_to_name(ret)); return(0);
    }

    FILE *f;
    f = fopen("/littlefs/CONFIG.txt", "wb");
    if (f == NULL)
    {
        fclose(f);
        esp_vfs_littlefs_unregister(conf.partition_label);
        printf("Failed to open file for writing\n");
        
    Release_Memory_mutex();
        return(0);
    }
    
    size_t rettt = fwrite((char*)eeprom_data, 1, sizeof(_eeprom), f);
    // printf("write ret %d", rettt);
    // esp_log_buffer_hex("WRITE", (char*)eeprom_data, sizeof(_flash));

    fclose(f);
    esp_vfs_littlefs_unregister(conf.partition_label);
    
    Release_Memory_mutex();
    return(1);
}

char read_eeprom(char* eeprom_data)
{
    GetMemoryMutex();
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        
    Release_Memory_mutex();
        printf("Failed to get LittleFS partition information (%s)\n", esp_err_to_name(ret));return(0);
    }

    FILE *f;
    f = fopen("/littlefs/CONFIG.txt", "r");
    if (f == NULL)
    {
        fclose(f);
        esp_vfs_littlefs_unregister(conf.partition_label);
            printf("Failed to open file for reading\n");
             Release_Memory_mutex();
            return(0);
    }        
    size_t rettt =fread((char*)eeprom_data, 1, sizeof(_eeprom),f);
    
    fclose(f);
    esp_vfs_littlefs_unregister(conf.partition_label);
    
    Release_Memory_mutex();
    return(1);
}

char validate_chksum_of_received_file(char *filename, int data_len)
{
    printf("file name %s, file size %d:%ld", filename,data_len, file_info.file_size);
    // int j=0, read_len = 0;
    int d = 0;
    char buff[1030]={0};
    char chksum=0,xor=0;
            
    int downloaded = 496,pkt_cnt=0,rx_data_count= 1024;
    while(1)
    {        
        pkt_cnt++;
        if(data_len> rx_data_count)
        {
            rx_data_count = 1024;
        }
        else rx_data_count = data_len;
        ReadDataFromFS((char*)filename,downloaded,&buff, rx_data_count);
        data_len -= rx_data_count;
        downloaded += rx_data_count;
        if(data_len == 0)
        {
            for (d=0; d<rx_data_count; d++)
            {
                chksum+= (char)buff[d];
                xor^=  (char)buff[d];
            }
            printf("\r\n pkt_cnt %d, download %d, chksum[%02X, %02X], chkxor[%02X, %02X]\r\n", pkt_cnt, downloaded, file_info.file_checksum,chksum,file_info.file_checkxor, xor);
            if((file_info.file_checksum == chksum) && (file_info.file_checkxor == xor))
            {
                return(1);
            }
            else
            {
                return(0);
            }         
        }
        else
        { 
            for (d=0; d<rx_data_count; d++)
            {
                chksum+= (char)buff[d];
                xor^=  (char)buff[d];
            }
        }
        
        
        if(pkt_cnt >=1001)// 1MB max file size allowed
        {
            break;
        }
        vTaskDelay(10/portTICK_PERIOD_MS);  
    }
    return(0);
}

// void save_event_data(_PB_DATA  pb_comp_data)
// {
//     Save_UnsendEvent_data((char*)&l_pb_data, sizeof(_PB_DATA));
// }   
