#ifndef SERIAL_FLASH_H_
#define SERIAL_FLASH_H_

enum
{
    OPEN = 0,
    FEED,
    CLOSE,
};

char Delete_memory_data();
char Delete_File(char* file);
char Erase_File_System(void); 
char read_eeprom(char* eeprom_data);
char write_eeprom(char* eeprom_data);
void Read_unsend_data(char* pBuffer, uint16_t data_size);
void Save_UnsendEvent_data(char* pBuffer, uint16_t data_size);
char validate_chksum_of_received_file(char *filename, int data_len);
char WriteDataToFS(char*file_name, int pos, char*pBuffer, int write_size);
char ReadDataFromFS(char*file_name, int pos, char*pBuffer, int read_size);
void write_memory_RDWRptr(uint32_t *wr_ptr, uint32_t *rd_ptr, uint8_t server);
void Read_memory_RDWRptr(uint32_t *wr_ptr, uint32_t *rd_ptr, uint8_t server);
// void save_event_data(PB_COMPRASION_PARAM  pb_comp_data);
#endif