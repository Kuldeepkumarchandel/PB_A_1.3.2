#ifndef OTA_H_
#define OTA_H_
extern bool OTA_status;
char OTA(char *file_name);
void do_esp_ota(char *fileName);
void convert_ascii_buff_to_hex_buff(char* data,int len);
void decrypt(char *Data);
void decrypt_1kb_pkt(char *Data);
void OTA_init();


// FAST OTA
uint8_t ota_init_setup();
char ota_final_setup();
char ota_data_write(char *data, uint16_t item_size);


#endif