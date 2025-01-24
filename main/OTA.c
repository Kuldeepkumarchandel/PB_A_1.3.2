#include "header.h"
bool OTA_status= false;

char ota_write_data[1100]={0};

void decrypt_1kb_pkt(char *l_ptr)
{
    decrypt(l_ptr);l_ptr+=256;
    decrypt(l_ptr);l_ptr+=256;
    decrypt(l_ptr);l_ptr+=256;
    decrypt(l_ptr);
}

void decrypt(char *l_ptr)
{
	char g_gps_data_bulk_buffer[512];
	memcpy((char*)(&g_gps_data_bulk_buffer[256]),(char*)(l_ptr),256);
	memcpy((char*)(&g_gps_data_bulk_buffer[0]),(char*)(&g_gps_data_bulk_buffer[448]),64);
	memcpy((char*)(&g_gps_data_bulk_buffer[64]),(char*)(&g_gps_data_bulk_buffer[384]),64);
	memcpy((char*)(&g_gps_data_bulk_buffer[128]),(char*)(&g_gps_data_bulk_buffer[320]),64);
	memcpy((char*)(&g_gps_data_bulk_buffer[192]),(char*)(&g_gps_data_bulk_buffer[256]),64);
	
	memcpy((char*)(&g_gps_data_bulk_buffer[256]),(char*)(&g_gps_data_bulk_buffer),64);
	memcpy((char*)(&g_gps_data_bulk_buffer[0]),(char*)(&g_gps_data_bulk_buffer[304]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[16]),(char*)(&g_gps_data_bulk_buffer[288]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[32]),(char*)(&g_gps_data_bulk_buffer[272]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[48]),(char*)(&g_gps_data_bulk_buffer[256]),16);
	
	memcpy((char*)(&g_gps_data_bulk_buffer[256]),(char*)(&g_gps_data_bulk_buffer[64]),64);
	memcpy((char*)(&g_gps_data_bulk_buffer[64]),(char*)(&g_gps_data_bulk_buffer[304]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[80]),(char*)(&g_gps_data_bulk_buffer[288]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[96]),(char*)(&g_gps_data_bulk_buffer[272]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[112]),(char*)(&g_gps_data_bulk_buffer[256]),16);
	
	memcpy((char*)(&g_gps_data_bulk_buffer[256]),(char*)(&g_gps_data_bulk_buffer[128]),64);
	memcpy((char*)(&g_gps_data_bulk_buffer[128]),(char*)(&g_gps_data_bulk_buffer[304]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[144]),(char*)(&g_gps_data_bulk_buffer[288]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[160]),(char*)(&g_gps_data_bulk_buffer[272]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[176]),(char*)(&g_gps_data_bulk_buffer[256]),16);
	
	memcpy((char*)(&g_gps_data_bulk_buffer[256]),(char*)(&g_gps_data_bulk_buffer[192]),64);
	memcpy((char*)(&g_gps_data_bulk_buffer[192]),(char*)(&g_gps_data_bulk_buffer[304]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[208]),(char*)(&g_gps_data_bulk_buffer[288]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[224]),(char*)(&g_gps_data_bulk_buffer[272]),16);
	memcpy((char*)(&g_gps_data_bulk_buffer[240]),(char*)(&g_gps_data_bulk_buffer[256]),16);
	memcpy((char*)(l_ptr),(char*)(g_gps_data_bulk_buffer),256);	
}

void OTA_end()
{

}

char OTA(char *file_name)
{    
    uint32_t file_actual_size = 0;
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;
   
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running)
    {
       printf("Configured OTA boot partition at offset 0x%08lX, but running from offset 0x%08lX",configured->address, running->address);
       printf("(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)\n");
    }
    
    // printf("Running partition type %d subtype %d (offset 0x%08x)",running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    // printf("Writing to partition subtype %d at offset 0x%x",update_partition->subtype, update_partition->address);
    
    assert(update_partition != NULL);

    /*deal with all receive packet*/
    bool image_header_was_checked = false;
   // ESP_LOGI("BLE OTA", "OK");  
    int downloaded=0,pkt_cnt=0,rx_data_count=496;
    while (1)
    {   
        pkt_cnt++;
        ReadDataFromFS((char*)file_name,downloaded,&ota_write_data, rx_data_count);
        downloaded += rx_data_count;
        printf("%s\r\n", ota_write_data);
        if(pkt_cnt==1)
        {
            Decode_file_information_from_file((char*)&ota_write_data);
            printf("Starting OTA Download .... filesize[%ld]\r\n",file_info.file_size);  
            file_actual_size = file_info.file_size + 496;
            rx_data_count = 1024;
            continue;
        }
        
        // ATL_Decrypt_1KBfile((s8 *)&ota_write_data);
        if(downloaded >= file_actual_size)
        {
            // printf("\r\nLast packet so clearing buffer downloaded[%ld] file_actual_size[%ld]\r\n",downloaded,file_actual_size);
            memset(&ota_write_data[(rx_data_count-(downloaded-file_actual_size))],0X00,((downloaded-file_actual_size)));
            downloaded = file_actual_size;
        }
        if(pkt_cnt >= 1024) // to break   
        {
            return 0;					   
        }
        
        if (image_header_was_checked == false) 
        {
            esp_app_desc_t new_app_info;
            // sprintf(trace_buf,"\r%d\n%d\n%d\n%d\n",rx_data_count, sizeof(esp_image_header_t), sizeof(esp_image_segment_header_t),sizeof(esp_app_desc_t));
            
            // printf("\r%d\n%d\n%d\n%d\n",rx_data_count, sizeof(esp_image_header_t), sizeof(esp_image_segment_header_t),sizeof(esp_app_desc_t));
            
            if (rx_data_count > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
            {
                // check current version with downloading
                memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                // printf("New firmware version: %s", new_app_info.version);
                // sprintf(trace_buf,"\r%d\n%d\n%d\n%d\n",rx_data_count, sizeof(esp_image_header_t), sizeof(esp_image_segment_header_t),sizeof(esp_app_desc_t));
               
                // printf(trace_buf);
                esp_app_desc_t running_app_info;
                if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
                {
                    // sprintf(trace_buf,"Running firmware version: %s", running_app_info.version);
                    // printf("Running firmware version: %s", running_app_info.version);
                }

                const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                esp_app_desc_t invalid_app_info;
                if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK)
                {
                    // sprintf(trace_buf,"Last invalid firmware version: %s", invalid_app_info.version);
                    // printf("Last invalid firmware version: %s", invalid_app_info.version);
                }

                // check current version with last invalid partition
                if (last_invalid_app != NULL) 
                {
                    if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) 
                    {
                        // printf("New version is the same as invalid version.");
                        // printf("Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                        // printf("The firmware has been rolled back to the previous version.");
                        // Leave OTA
                        OTA_end();
                        // OTA_status=OFF;
                        return(0);
                    }
                }
                vTaskDelay(2000/portTICK_PERIOD_MS);
                image_header_was_checked = true;

                err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                if (err != ESP_OK)
                {
                    //send_rs232data("esp_ota_begin failed", 0xFFFF);
                    // printf("esp_ota_begin failed (%s)", esp_err_to_name(err));
                    OTA_end();
                    // OTA_status=OFF;
                    return(0);
                }
                //send_rs232data("esp_ota_begin succeeded",0XFFFF);
            } 
            else 
            {
                printf("received package is not fit len"); 
                // OTA_status=OFF;
                return(0); 
                
            }
        }
        
        err = esp_ota_write(update_handle, (const void *)ota_write_data, rx_data_count);                
        if (err != ESP_OK) 
        {
            OTA_end();
            // OTA_status=OFF;
            return(0);
        } 
        else
        {
            // printf("ok write\n");
        }    
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if(downloaded==file_actual_size)break;
   }

    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    if (esp_ota_end(update_handle) != ESP_OK) 
    {
        // OTA_status=OFF;
        printf("\r\nESP_OTA_END!=ESP_OK\r\n");
        return(0);
    }
    else
    {
        // printf("\r\nESP_OTA_END==ESP_OK\r\n");
    }
    
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) 
    {
        printf("OTA_status=OFF1;");
        // OTA_status=OFF;
        return(0);
    }
    else
    {
        // printf("ssssssssssssucess");
        return(1);
    }
    
}

void convert_ascii_buff_to_hex_buff(char* data,int len)
{
    for(int i=0,j=0;i<len;i=i+2,j++)
    {
        data[j] = ((ascii2hex(data[i])<<4) | ascii2hex(data[i+1]));
    }
}


void do_esp_ota(char *fileName)
{
    char ret = 0;
    if(OTA((char *)fileName))
    {
        Send_BLEdata("Firmware Upgrade Successfully", sizeof("Firmware Upgrade Successfully"));
        PB_esp_restart();
    }
    else
    {
        Send_BLEdata("Firmware Upgrade Failed", sizeof("Firmware Upgrade Failed"));
    }
}



uint32_t recv_len = 0;
esp_partition_t *partition_ptr = NULL;
esp_partition_t partition;
const esp_partition_t *next_partition = NULL;
esp_ota_handle_t out_handle = 0;


uint8_t ota_init_setup()
{
    recv_len = 0;
    out_handle = 0;
    ESP_LOGI("OTA", "ota task startup");
    // search ota partition
    partition_ptr = (esp_partition_t *)esp_ota_get_boot_partition();
    if (partition_ptr == NULL) {
        ESP_LOGE("OTA", "boot partition NULL!\r\n");
        goto OTA_ERROR;
    }
    if (partition_ptr->type != ESP_PARTITION_TYPE_APP) {
        ESP_LOGE("OTA", "esp_current_partition->type != ESP_PARTITION_TYPE_APP\r\n");
        goto OTA_ERROR;
    }

    if (partition_ptr->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY) {
        partition.subtype = ESP_PARTITION_SUBTYPE_APP_OTA_0;
    } 
    else 
    {
        next_partition = esp_ota_get_next_update_partition(partition_ptr);
        if (next_partition) {
            partition.subtype = next_partition->subtype;
        } else {
            partition.subtype = ESP_PARTITION_SUBTYPE_APP_OTA_0;
        }
    }
    partition.type = ESP_PARTITION_TYPE_APP;

    partition_ptr = (esp_partition_t *)esp_partition_find_first(partition.type, partition.subtype, NULL);
    if (partition_ptr == NULL) 
    {
        ESP_LOGE("OTA", "partition NULL!\r\n");
        goto OTA_ERROR;
    }

    memcpy(&partition, partition_ptr, sizeof(esp_partition_t));
    if (esp_ota_begin(&partition, OTA_SIZE_UNKNOWN, &out_handle) != ESP_OK) {
        ESP_LOGE("OTA", "esp_ota_begin failed!\r\n");
        goto OTA_ERROR;
    }
    ESP_LOGI("OTA", "esp_ota_begin success\r\n");
    return (1);


    OTA_ERROR:
        ESP_LOGE("OTA", "OTA failed, %ld", file_info.file_size);
    
    return (0);

}

char ota_data_write(char *data, uint16_t item_size)
{
   
    if (item_size != 0)
    {
        if (esp_ota_write(out_handle, (const void *)data, item_size) != ESP_OK) 
        {
            ESP_LOGE("OTA", "esp_ota_write failed!\r\n");
            return 0;
        }
        return 1;
    }
    return 0;
}

char ota_final_setup()
{
    if (esp_ota_end(out_handle) != ESP_OK)
    {
        ESP_LOGE("OTA", "esp_ota_end failed!\r\n");
        return 0; 
    }
    ESP_LOGE("OTA", "esp_ota_end success!\r\n");
    if (esp_ota_set_boot_partition(&partition) != ESP_OK)
    {
        ESP_LOGE("OTA", "esp_ota_set_boot_partition failed!\r\n");
        return 0; 
    }
    ESP_LOGE("OTA", "esp_ota_set_boot_partition SUCCESS!\r\n");
    return 1; 
}
      
