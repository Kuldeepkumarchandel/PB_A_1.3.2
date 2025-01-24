#ifndef BLE_H_
#define BLE_H_

    extern bool Is_ble_connected;
    #define GATTS_SERVICE_UUID_TEST_A   0x00FF
    #define GATTS_CHAR_UUID_TEST_A      0xFF01
    #define GATTS_DESCR_UUID_TEST_A     0x3333
    #define GATTS_NUM_HANDLE_TEST_A     4

    #define PROFILE_NUM 1
    #define PROFILE_A_APP_ID 0
    #define GATTS_DEMO_CHAR_VAL_LEN_MAX 0x40
    
    typedef enum
    {
        A_NONE = 0,
        A_PANIC,
        A_BATTERY,
    }_ALERT_TYPE;


    typedef struct {
        char deviceID[4];
        char temperature1;
        char temperature2;
        char humidity;
        char battery_percentage;
        char ALERT_TYPE;
        char ALERT_INFO1;
        char ALERT_INFO2;
        char mac[6];
    } PB_Data;

    extern volatile PB_Data PB_reading; 
    
    typedef struct
    {
        uint8_t file_name[20];
        uint8_t file_checksum;
        uint8_t file_checkxor;
        uint32_t file_size;
        uint16_t packet_data_size;
        uint16_t files_total_packet;
        // size_t packet_checksum;
        // size_t packet_checkxor;
    } FileInfo;
    extern volatile FileInfo file_info;
    extern char Ble_rxbuff[520];
    extern uint16_t Ble_length;
    void Handle_BLE_Data();
    void handle_configuration_pkt(char* buff);
    void Bluetooth_data_advertisement();
    void Bluetooth_deinit();
    void Send_BLEdata(char *buffer, uint16_t len);
    void Bluetooth_init();
    bool GetBluetooth_ConnectionStatus();
    void GetBluetooth_MAC_ADDR(uint8_t* MAC_ADDR);
    void Set_new_rtc_time(time_t epochTime);
    char calc_chksum(char * ptr, int len);
    char Decode_save_data(char *data);
    void fatch_data(char *data) ;
    uint8_t Decode_first_packet_information();
    void Decode_file_information_from_file(char *data);
    void Decode_Filename(char data, char *file_Name);
    void remove_all_bonded_devices();
    void ble_disconnect_connected_device();
    void fatch_preset_configure_data(char *pbuffer);
    char handle_presetconfigure_configuration_pkt(char *pbuffer);
#endif  // BLE_H_