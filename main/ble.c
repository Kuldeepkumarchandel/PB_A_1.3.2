
#include "header.h"
#include <time.h>
// #define DEVICE_BLE_NAME            "PILL_BOX"

#ifdef CONFIG_BT_ENABLED

    // BLE connection parameters and flags
    uint32_t passkey = 123456;                          // Default passkey for BLE pairing
    int8_t bluetooth_name[20] = {0};                    // Device name buffer
    bool Is_ble_connected = false;                      // Flag indicating if BLE is connected
    static bool Is_ble_init = false;                    // Flag indicating if BLE is initialized
    static uint16_t s_conn_id = 0xffff;                // Connection ID
    static esp_gatt_if_t s_gatts_if = 0xff;            // GATT interface
    static esp_gatt_char_prop_t a_property = 0;         // GATT characteristic properties

    // File transfer related variables
    volatile FileInfo file_info;                        // Structure to hold file transfer info
    static uint8_t dummy_data[] = {0x11,0x22,0x33};    // Dummy data for testing
    char Ble_rxbuff[520] = {0};                        // Buffer for received BLE data
    uint32_t g_Ble_rxbuff_subsc = 0;                   // Subscription counter for BLE buffer
    uint16_t Ble_length = 0;                           // Length of received BLE data
    uint32_t Total_packet_recived = 0;                 // Counter for total received packets

    // GATT characteristic value
    static esp_attr_value_t gatts_demo_char1_val = {    // GATT characteristic value structure
        .attr_max_len = GATTS_DEMO_CHAR_VAL_LEN_MAX,   // Maximum length of attribute
        .attr_len     = sizeof(dummy_data),            // Current length of attribute
        .attr_value   = dummy_data,                    // Pointer to attribute value
    };
    
    // Function declarations
    static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

    /**
     * @brief Disconnects the currently connected BLE device.
     * 
     * @return esp_err_t ESP_OK if disconnection is initiated successfully, error code otherwise.
    */
    
    void ble_disconnect_connected_device() 
    {
        // Get the list of bonded devices
        int dev_num = esp_ble_get_bond_device_num();
        if (dev_num == 0) 
        {
            printf("No bonded devices found.\n");
            return;
        }

        esp_ble_bond_dev_t *bonded_devices = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
        esp_ble_get_bond_device_list(&dev_num, bonded_devices);

        // Remove each bonded device
        for (int i = 0; i < dev_num; i++)
        {
            esp_err_t err = esp_ble_gap_disconnect(bonded_devices[i].bd_addr);
            if (err == ESP_OK)
            {
                printf("Removed bonded device: %02x:%02x:%02x:%02x:%02x:%02x\n",bonded_devices[i].bd_addr[0], bonded_devices[i].bd_addr[1],bonded_devices[i].bd_addr[2], bonded_devices[i].bd_addr[3],bonded_devices[i].bd_addr[4], bonded_devices[i].bd_addr[5]);
            }
            else
            {
                printf("Failed to remove bonded device.\n");
            }
        }
        free(bonded_devices);
    }



    /**
     * @brief Removes all bonded BLE devices from memory
     * 
     * This function:
     * 1. Gets the number of bonded devices
     * 2. Allocates memory to store bonded device list
     * 3. Gets the list of bonded devices
     * 4. Removes each bonded device one by one
     * 5. Frees allocated memory
     */
   void remove_all_bonded_devices()
   {
        // Get the list of bonded devices
        int dev_num = esp_ble_get_bond_device_num();
        if (dev_num == 0) {
            printf("No bonded devices found.\n");
            return;
        }

        esp_ble_bond_dev_t *bonded_devices = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
        esp_ble_get_bond_device_list(&dev_num, bonded_devices);

        // Remove each bonded device
        for (int i = 0; i < dev_num; i++)
        {
            esp_err_t err = esp_ble_remove_bond_device(bonded_devices[i].bd_addr);
            if (err == ESP_OK) {
                printf("Removed bonded device: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    bonded_devices[i].bd_addr[0], bonded_devices[i].bd_addr[1],
                    bonded_devices[i].bd_addr[2], bonded_devices[i].bd_addr[3],
                    bonded_devices[i].bd_addr[4], bonded_devices[i].bd_addr[5]);
            } else {
                printf("Failed to remove bonded device.\n");
            }
        }
        free(bonded_devices);
    }

    // Raw advertising data if configured
    #ifdef CONFIG_SET_RAW_ADV_DATA
        static uint8_t raw_adv_data[] = {
                0x02, 0x01, 0x06,
                0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd
        };
        static uint8_t raw_scan_rsp_data[] = {
                0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41, 0x54, 0x54, 0x53, 0x5f, 0x44,
                0x45, 0x4d, 0x4f
        };
    #else

    // Service UUID for BLE service
    static uint8_t adv_service_uuid128[16] = {
        /* LSB <--------------------------------------------------------------------------------> MSB */
        //first uuid, 16bit, [12],[13] is the value
        0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
        //second uuid, 32bit, [12], [13], [14], [15] is the value
        // 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
    };

        // Variables for sensor data and alerts
        char TEMP1 = 25, TEMP2 = 78, HUMD = 35, BAT_PER = 0,ALERT_TYPE = 0,ALERT_INFO1 = 0,ALERT_INFO2=0;
        
        // Structure to hold pill box data
        volatile PB_Data PB_reading; 
        
        // Advertising data configuration
        esp_ble_adv_data_t adv_data =
        {
            .set_scan_rsp = false,
            .include_name = true,
            .appearance = 0x00,
            .service_uuid_len = sizeof(adv_service_uuid128),
            .p_service_uuid = adv_service_uuid128,
            .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
        };

        // Scan response data configuration
        static esp_ble_adv_data_t scan_rsp_data = {
            .set_scan_rsp = true,
            .manufacturer_len =  sizeof(PB_Data),
            .p_manufacturer_data = (uint8_t*)&PB_reading,
        };

    #endif /* CONFIG_SET_RAW_ADV_DATA */

    // Advertising parameters configuration
    static esp_ble_adv_params_t adv_params = {
        .adv_int_min        = 0x20,
        .adv_int_max        = 0x40,
        .adv_type           = ADV_TYPE_IND,
        .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
        .channel_map        = ADV_CHNL_ALL,
        .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };

    // GATT profile structure
    struct gatts_profile_inst {
        esp_gatts_cb_t gatts_cb;
        uint16_t gatts_if;
        uint16_t app_id;
        uint16_t conn_id;
        uint16_t service_handle;
        esp_gatt_srvc_id_t service_id;
        uint16_t char_handle;
        esp_bt_uuid_t char_uuid;
        esp_gatt_perm_t perm;
        esp_gatt_char_prop_t property;
        uint16_t descr_handle;
        esp_bt_uuid_t descr_uuid;
    };

    // Profile table for GATT server
    struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
        [PROFILE_A_APP_ID] = {
            .gatts_cb = gatts_profile_a_event_handler,
            .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
        },
    };

    /**
     * @brief GAP event handler for BLE events
     * 
     * Handles various BLE GAP events including:
     * - Advertisement data setting
     * - Scan response data setting  
     * - Advertisement start
     * - Passkey notification
     * - Authentication completion
     * - Connection parameter updates
     *
     * @param event GAP event type
     * @param param Event parameters
     */
    static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
    {
        switch (event) {
            case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
                    esp_ble_gap_start_advertising(&adv_params);
                break;
            case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
                    esp_ble_gap_start_advertising(&adv_params);
                break;

           
            case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
                //advertising start complete event to indicate advertising start successfully or failed
                if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                    ESP_LOGE("BLE", "Advertising start failed\n");
                }
                break;

            case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  ///the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
                ///show the passkey number to the user to input it in the peer device.
                ESP_LOGI("BLE", "The passkey Notify number:%06ld", param->ble_security.key_notif.passkey);
                break;

            case ESP_GAP_BLE_AUTH_CMPL_EVT: {
                esp_bd_addr_t bd_addr;
                memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
                ESP_LOGI("BLE", "remote BD_ADDR: %08x%04x",(bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) +   
                    bd_addr[3], (bd_addr[4] << 8) + bd_addr[5]);
                ESP_LOGI("BLE", "address type = %d",param->ble_security.auth_cmpl.addr_type);  
                ESP_LOGI("BLE", "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
                if(!param->ble_security.auth_cmpl.success) {
                    ESP_LOGI("BLE", "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
                }
                break;
            }

            case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
                printf("update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d \n",
                        param->update_conn_params.status,
                        param->update_conn_params.min_int,
                        param->update_conn_params.max_int,
                        param->update_conn_params.conn_int,
                        param->update_conn_params.latency,
                        param->update_conn_params.timeout);
            break;

            default:
            break;

        }
        
    }

    /**
     * @brief GATT profile event handler for profile A
     * 
     * Handles GATT server events for profile A including:
     * - Registration
     * - Read requests
     * - Write requests
     * - MTU exchange
     * - Service creation
     * - Characteristic addition
     * - Descriptor addition
     * - Service start
     * - Client connect/disconnect
     * - Confirmation events
     *
     * @param event GATT event type
     * @param gatts_if GATT interface
     * @param param Event parameters
     */
    static void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
    {
        switch (event) 
        {
            case ESP_GATTS_REG_EVT:
                printf( "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
                gl_profile_tab[PROFILE_A_APP_ID].service_id.is_primary = true;
                gl_profile_tab[PROFILE_A_APP_ID].service_id.id.inst_id = 0x00;
                gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
                gl_profile_tab[PROFILE_A_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID_TEST_A;

                // Set device name
                esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name((const char *)eeprom.device_name);
                if (set_dev_name_ret){
                    printf( "set device name failed, error code = %x\n", set_dev_name_ret);
                }
                
                // Configure advertising data
                Bluetooth_data_advertisement();
                
                // Configure scan response data
                esp_err_t ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
                if (ret){
                    printf( "config scan response data failed, error code = %x\n", ret);
                }
        
                esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_A_APP_ID].service_id, GATTS_NUM_HANDLE_TEST_A);
            break;

            case ESP_GATTS_READ_EVT: {
                printf("GATT_READ_EVT, conn_id %d, trans_id %ld, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
                esp_gatt_rsp_t rsp;
                memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
                rsp.attr_value.handle = param->read.handle;
                rsp.attr_value.len = 4;
                rsp.attr_value.value[0] = 0xaa;
                rsp.attr_value.value[1] = 0xbb;
                rsp.attr_value.value[2] = 0xcc;
                rsp.attr_value.value[3] = 0xdd;
                esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                            ESP_GATT_OK, &rsp);
                break;
            }

            case ESP_GATTS_WRITE_EVT: {
                // Handle write event from client
                memset(Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                if (gl_profile_tab[PROFILE_A_APP_ID].descr_handle == param->write.handle && param->write.len == 2){
                    s_conn_id = param->write.conn_id;
                    s_gatts_if = gatts_if;
                }
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, 0, NULL);
                memcpy((char*)&Ble_rxbuff[0],(uint8_t*)param->write.value, param->write.len);            
                Ble_length = param->write.len;
                f.g_ble_rcvd_F = 1;          
                break;
            }
    
            case ESP_GATTS_MTU_EVT:
                printf( "ESP_GATTS_MTU_EVT, MTU %d\n", param->mtu.mtu);
                break;
        
            case ESP_GATTS_CREATE_EVT:
                printf( "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status, param->create.service_handle);
                                                                                                                
                gl_profile_tab[PROFILE_A_APP_ID].service_handle = param->create.service_handle;
                gl_profile_tab[PROFILE_A_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
                gl_profile_tab[PROFILE_A_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID_TEST_A;

                esp_ble_gatts_start_service(gl_profile_tab[PROFILE_A_APP_ID].service_handle);
                a_property = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
                esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].char_uuid,
                                                                ESP_GATT_PERM_WRITE_ENCRYPTED | ESP_GATT_PERM_READ_ENCRYPTED,
                                                                a_property,
                                                                &gatts_demo_char1_val, NULL);
                if (add_char_ret){
                    printf( "add char failed, error code =%x\n",add_char_ret);		  
                }																											   
                break;
                
            case ESP_GATTS_ADD_CHAR_EVT: {
                printf( "ADD_CHAR_EVT, status %d,  attr_handle %d, service_handle %d\n",
                        param->add_char.status, param->add_char.attr_handle, param->add_char.service_handle);
                gl_profile_tab[PROFILE_A_APP_ID].char_handle = param->add_char.attr_handle;
                gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
                gl_profile_tab[PROFILE_A_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
                
                esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_A_APP_ID].service_handle, &gl_profile_tab[PROFILE_A_APP_ID].descr_uuid,
                                                                        ESP_GATT_PERM_WRITE_ENCRYPTED, NULL, NULL);
                if (add_descr_ret){
                    printf( "add char descr failed, error code =%x\n", add_descr_ret);
                }
                break;
            }
            case ESP_GATTS_ADD_CHAR_DESCR_EVT:
                gl_profile_tab[PROFILE_A_APP_ID].descr_handle = param->add_char_descr.attr_handle;
                printf( "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
                        param->add_char_descr.status, param->add_char_descr.attr_handle, param->add_char_descr.service_handle);
            break;

            case ESP_GATTS_START_EVT:
                printf( "SERVICE_START_EVT, status %d, service_handle %d\n",
                        param->start.status, param->start.service_handle);
                break;

            case ESP_GATTS_CONNECT_EVT: {
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
            Is_ble_connected = true; f.g_reset_counter_and_flag_F = 1;
            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
            conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
            conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
            printf( "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:\n",
                    param->connect.conn_id,
                    param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                    param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
            gl_profile_tab[PROFILE_A_APP_ID].conn_id = param->connect.conn_id;
            //start sent the update connection parameters to the peer device.
            // esp_ble_gap_update_conn_params(&conn_params);
            // esp_ble_gap_start_advertising(&adv_params);

            break;
        }
        
        case ESP_GATTS_DISCONNECT_EVT:
            Is_ble_connected  = false;
            printf( "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x\n", param->disconnect.reason);
            esp_ble_gap_start_advertising(&adv_params);
        break;
        
        case ESP_GATTS_CONF_EVT:
            // printf( "ESP_GATTS_CONF_EVT, status %d attr_handle %d\n", param->conf.status, param->conf.handle);
            if (param->conf.status != ESP_GATT_OK){
                // esp_log_buffer_hex("BLE", param->conf.value, param->conf.len);
            }
        break;
    
        default:
            break;
        }
    }
    
    /**
     * @brief Main GATT server event handler
     * 
     * This function:
     * 1. Handles registration events for GATT profiles
     * 2. Routes events to appropriate profile handlers based on interface
     *
     * @param event GATT event type
     * @param gatts_if GATT interface
     * @param param Event parameters
     */
    static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
    {
        /* If event is register event, store the gatts_if for each profile */
        if (event == ESP_GATTS_REG_EVT) {
            if (param->reg.status == ESP_GATT_OK) {
                gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
            } 
            
            else {
                printf( "Reg app failed, app_id %04x, status %d\n",
                        param->reg.app_id,
                        param->reg.status);
                return;
            }
        }

        /* If the gatts_if equal to profile A, call profile A cb handler,
        * so here call each profile's callback */
        do {
            int idx;
            for (idx = 0; idx < PROFILE_NUM; idx++) {
                /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                if (gatts_if == ESP_GATT_IF_NONE || gatts_if == gl_profile_tab[idx].gatts_if) {
                    if (gl_profile_tab[idx].gatts_cb) {
                        gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                    }
                }
            }
        } while (0);
    }

    /**
     * @brief Send data over BLE
     * 
     * Sends data to connected BLE client using GATT notification
     *
     * @param buffer Data buffer to send
     * @param len Length of data
     */
    void Send_BLEdata(char* buffer, uint16_t len)
    {
        if(len == (uint16_t)0xFFFF)len = strlen((const char*)buffer);
        if(Is_ble_connected)
        {
            // ESP_LOGI("Send_BLEdata ", "%s", buffer);
            esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id, gl_profile_tab[PROFILE_A_APP_ID].char_handle, len, (uint8_t *)buffer, false);
        }
    }

    /**
     * @brief Get BLE connection status
     * 
     * @return bool Connection status (true if connected)
     */
    bool GetBluetooth_ConnectionStatus()
    {
        return Is_ble_connected;
    }

    /**
     * @brief Calculate checksum for data
     * 
     * Calculates simple additive checksum of data buffer
     *
     * @param ptr Data pointer
     * @param len Data length
     * @return char Calculated checksum
     */
    char calc_chksum(char * ptr, int len)
    {
        unsigned int i = 0;
        char chksum=0;
        for(i=0;i<len;i++)
        {
            chksum += *ptr++;
        }
        if(chksum == 0x00)chksum = 0xFE;
        return(chksum);
    }

    /**
     * @brief Fetch device configuration data
     * 
     * Retrieves and formats device configuration data including:
     * - Device name
     * - BLE passkey
     * - Ping interval
     * - RTC epoch
     * - Timer configurations
     * - Alarm settings
     *
     * @param data Buffer to store data
     */
    void fatch_data(char *data) //"CFG,DEVICE_TEST,6619,300M,290523142532,28- timers,9,2,time_mode,45,&";
    {
        char temp[25] = {0};
        char chksum = 0;
        memset(data, 0x00, sizeof(Ble_rxbuff));memset((char*)temp, 0x00, sizeof(temp)); sprintf(data,"%s","CFG,");
        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%s,",eeprom.device_name); strcat(data, temp);
        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%d,",eeprom.BLE_PASS);    strcat(data, temp);
        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%s_%s,",HW_VERSION,SW_VERSION);strcat(data, temp);
        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%03d",eeprom.ping_interval);strcat(data, temp);
        if(eeprom.ping_interval_unit == G_SEC)
        {
            strcat(data, "S,");
        }
        if(eeprom.ping_interval_unit == G_MIN)
        {
            strcat(data, "M,");
        }
        if(eeprom.ping_interval_unit == G_HH)
        {
            strcat(data, "H,");
        }
        memset((char*)temp, 0x00, sizeof(temp));
        eeprom.rtc_epoch = 0;
        READ_RTC_and_GET_Epoch(&eeprom.rtc_epoch);
        sprintf(temp,"%ld,",eeprom.rtc_epoch);strcat(data, temp);            // we will not give 
        sprintf(temp,"%03d,",eeprom.total_timers_configured);strcat(data, temp);   
        for (int i = 0; i < eeprom.total_timers_configured; i++)
        { 
            memset((char*)temp, 0x00, sizeof(temp));
            sprintf(temp,"%02d-%10ld",(eeprom.config_alarm_time[i].Box_ID), eeprom.config_alarm_time[i].timer_epoch);
            strcat(data, temp);
            strcat(data, ",");
        }
     
       memset(temp, 0x00, sizeof(temp));    sprintf(temp,"%d,", eeprom.alarm_volume);           strcat(data, temp);
        // // don't know about the FR byte handling 
       memset(temp, 0x00, sizeof(temp));    sprintf(temp,"%d,",eeprom.time_mode);               strcat(data, temp);
       memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%03d",eeprom.alarm_gap);strcat(data, temp);
        if(eeprom.alarm_gap_unit == G_SEC)
        {
            strcat(data, "S,");
        }
        if(eeprom.alarm_gap_unit == G_MIN)
        {
            strcat(data, "M,");
        }
        if(eeprom.alarm_gap_unit == G_HH)
        {
            strcat(data, "H,");
        }
        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%03d",eeprom.TADE_interval);strcat(data, temp);
        if(eeprom.TADE_interval_unit == G_SEC)
        {
            strcat(data, "S,");
        }
        if(eeprom.TADE_interval_unit == G_MIN)
        {
            strcat(data, "M,");
        }
        if(eeprom.TADE_interval_unit == G_HH)
        {
            strcat(data, "H,");
        }
        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%03d",eeprom.panic_interval);strcat(data, temp);
        if(eeprom.panic_interval_unit == G_SEC)
        {
            strcat(data, "S,");
        }
        if(eeprom.panic_interval_unit == G_MIN)
        {
            strcat(data, "M,");
        }
        if(eeprom.panic_interval_unit == G_HH)
        {
            strcat(data, "H,");
        }
        
        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%03d",eeprom.ble_connection_timeout);strcat(data, temp);
        if(eeprom.ble_connection_timeout_unit == G_SEC)
        {
            strcat(data, "S,");
        }
        if(eeprom.ble_connection_timeout_unit == G_MIN)
        {
            strcat(data, "M,");
        }
        if(eeprom.ble_connection_timeout_unit == G_HH)
        {
            strcat(data, "H,");
        }

        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%03d",eeprom.backlight_Off_timeout);strcat(data, temp);
        if(eeprom.backlight_Off_timeout_unit == G_SEC)
        {
            strcat(data, "S,");
        }
        if(eeprom.backlight_Off_timeout_unit == G_MIN)
        {
            strcat(data, "M,");
        }
        if(eeprom.backlight_Off_timeout_unit == G_HH)
        {
            strcat(data, "H,");
        }

        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%03d",eeprom.display_timeout);strcat(data, temp);
        if(eeprom.display_timeout_unit == G_SEC)
        {
            strcat(data, "S,");
        }
        if(eeprom.display_timeout_unit == G_MIN)
        {
            strcat(data, "M,");
        }
        if(eeprom.display_timeout_unit == G_HH)
        {
            strcat(data, "H,");
        }

        memset((char*)temp, 0x00, sizeof(temp));sprintf(temp,"%02d,",eeprom.battery_alert_threshold);strcat(data, temp);
        //  FR BYTE
       memset(temp, 0x00, sizeof(temp));    sprintf(temp,"%d,", 0);                             strcat(data, temp);
        // printf("len %d \t", strlen(data));
       chksum = calc_chksum(data, strlen(data));  
       memset(temp, 0x00, sizeof(temp));    sprintf(temp,"%02x,&", chksum);      strcat(data, temp); 
        printf("data %s \n", data);
        int data_len = strlen((char*)data);
        if(data_len<=480)
        {
            Send_BLEdata((char*)data, data_len);
        }
        else
        {
            char temp[512]={0};
            for(int i=0;i<4;i++)
            {
                memset((char*)temp,0x00,sizeof(temp));
                if(data_len>=480)
                {
                    strcpy(temp,"$$");temp[2]=0x30+i;temp[3]=',';
                    strncat(temp,(char*)&data[i*480],480);
                    Send_BLEdata((char*)temp, 0xFFFF);
                    data_len-=480;
                    vTaskDelay(200/portTICK_PERIOD_MS);
                }
                else
                {
                    strcpy(temp,"$$");temp[2]=0x30+i;temp[3]=',';
                    strncat(temp,(char*)&data[i*480],strlen((char*)&data[i*480]));
                    strcat(temp,",%%");
                    Send_BLEdata((char*)temp, 0xFFFF);
                    break;                    
                }
            }            
        }
    }

    void Set_new_rtc_time(time_t epochTime)
    {        
        char dt, second, minute, hour,month,year;
        char time[10] = {0}, date[15] = {0}, temp[5] = {0};
        convert_epoch_into_gmttime_and_date(epochTime,(char*)&date, (char*)&time);

        // rtc_hour = ((a[0]-0x30)<<4) | (a[1]-0x30);
        //rtc_mm = ((b[0]-0x30)<<4) | (b[1]-0x30);

        second  =((time[4] - '0') << 4) | (time[5] - '0');
        minute = ((time[2] - '0') << 4) | (time[3] - '0');
        hour  = ((time[0] - '0') << 4) | (time[1] - '0');


        dt  = ((date[0] - '0') <<4) | (date[1] - '0');
        month = ((date[2] - '0') <<4) | (date[3] - '0');
        year  = ((date[4] - '0') <<4) | (date[5] - '0');
        
        printf(" [Time %s=%x:%x:%x, %d:%d:%d], [Date=%s %x:%x:%x, %d:%d:%d]\r\n",time, hour,minute,second,hour,minute,second,date,dt,month,year,dt,month,year);
        

        time_t now = epochTime;
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        // year = timeinfo.tm_year - 100;
        // year = timeinfo.tm_year + 1900;
        // month  = timeinfo.tm_mon + 1;
        // date    = timeinfo.tm_mday;
        // hour   = timeinfo.tm_hour;
        // minute = timeinfo.tm_min;
        // second = timeinfo.tm_sec;
        char wday   = timeinfo.tm_wday;  // Day of the week (Sunday = 0)
        // int  yday   = timeinfo.tm_yday;  // Day of the year (0 to 365/366)
        // decode_day(wday);
        // printf("[Week-Day=%s], [Time=%x:%x:%x, %d:%d:%d], [Date=%x:%x:%x, %d:%d:%d]\r\n",g_day, hour,minute,second, hour,minute,second,day,month,year,day,month,year);
        set_time(wday,second,minute,hour,dt,month,year);
    }

    /**
     * The function `Decode_save_data` decodes and saves data received via BLE communication.
     * 
     * @param data The `data` parameter is a character array that contains the received data from a BLE
     * device.
     */
     //char tempp[] = "CFG,PB,691900,100M,1709668394,32,01-1702432799,02-1702432796,03-1702432797,04-1702432798,05-1702432799,06-1702432760,07-1702432761,08-1702432762,09-1702432763,10-1702432764,11-1702432769,12-1702432766,13-1702432767,14-1702432768,19-1702432769,16-1702432770,17-1702432771,18-1702432772,19-1702432773,20-1702432774,21-1702432779,22-1702432776,23-1702432777,24-1702432778,25-1702432779,26-1702432780,27-1702432781,28-1702432782,6,0,110S,220M,330H,5,12,&";

    char Decode_save_data(char *data)
    {
        // printf(" BLE RECEIVED DATA : %s \r\n", data);
        char temp[20] = {0};
        int i = 0, count = 0;
        char *l_ptr;
        int unit = 0;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",1,0,0,0,16);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            if (strncmp(temp, eeprom.device_name,15) != 0)
            {
                f.g_change_device_name_pass_F = 1;
            }
            memcpy((char *)&eeprom.device_name, temp,15);
        } 
        ESP_LOGI("Device Name  = ", "%s,", temp);  
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",2,0,0,0,8);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            if(eeprom.BLE_PASS != atoi(temp))
            {
                f.g_change_device_name_pass_F = 2;
            }
            eeprom.BLE_PASS = atoi(temp);
            if(eeprom.BLE_PASS < 1)
            {
                return(0);
            }
        }   
        ESP_LOGI("BLE_PASS  = ", "%s,", temp); 
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",3,0,0,0,5);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.ping_interval =  (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.ping_interval_unit =  G_SEC;unit =1;}
            else if( temp[3] == 'M') {  eeprom.ping_interval_unit =  G_MIN;unit = 60;}
            else if( temp[3] == 'H') {  eeprom.ping_interval_unit =  G_HH; unit = 3600;}   
            eeprom.ping_interval_sec = eeprom.ping_interval*unit;
        }
        ESP_LOGI("ping interval=", "%s,", temp);
        
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",4,0,0,0,10);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.rtc_epoch = atoi(temp);
            Set_new_rtc_time(eeprom.rtc_epoch);
        }
        printf("eeprom.rtc_epoch %ld\n",eeprom.rtc_epoch);
        char flag=0;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",5,0,0,0,10);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            unsigned char tem=0;
            tem = (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if(tem>121)
            {
                flag = 2;
            }
            else{
                eeprom.total_timers_configured = tem;
            }
        }
        else{
            flag=1;
        }
        int cnt_temp=5;
        printf("eeprom.total_timers_configured  %03d\r\n",eeprom.total_timers_configured);
        for (int i = 0; i < eeprom.total_timers_configured; i++)
        {
            if(flag==1)break;
            cnt_temp=i+6;
            memset((char*)temp, 0x00, 20);
            get_parameter_from_data((char*)&temp, (char*)data, ",",i+6, 0, 0, 0, 15);
            eeprom.config_alarm_time[i].Box_ID = (temp[0] - '0') * 10 + (temp[1] - '0');
            if(strstr(model_info,"PB28"))
            {
                    if(eeprom.config_alarm_time[i].Box_ID>27){flag=2;break;}
            }
            else if(strstr(model_info,"PB29"))
            {
                    if(eeprom.config_alarm_time[i].Box_ID>28){flag=2;break;}
            }
           
           
           
            //eeprom.config_alarm_time[i].Box_ID -= 1;
            if(((temp[3] == '-') && (temp[4] == '2')))
            {
                eeprom.config_alarm_time[i].timer_epoch = 0 ;  
                eeprom.config_alarm_time[i].timer_result_epoch=0;
                eeprom.config_alarm_time[i].alarm_gap_epoch = 0; 
                eeprom.config_alarm_time[i].total_alarm_duratition_epoch = 0;
                eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS=0;
                eeprom.config_alarm_time[i].alarm_repetition_cnt=0;
                eeprom.config_alarm_time[i].last_triggred_time = 0;
                eeprom.config_alarm_time[i].buffer_period_epoch = 0;
            }
            else if(!((temp[3] == '-') && (temp[4] == '1')))
            {
                l_ptr = &temp[3];
                eeprom.config_alarm_time[i].timer_epoch = atoi(l_ptr); 
                eeprom.config_alarm_time[i].timer_result_epoch=0;
                eeprom.config_alarm_time[i].alarm_gap_epoch = eeprom.alarm_gap_sec; 
                eeprom.config_alarm_time[i].total_alarm_duratition_epoch = eeprom.TADE_interval_sec;
                eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS=1;
                eeprom.config_alarm_time[i].alarm_repetition_cnt=0;
                eeprom.config_alarm_time[i].last_triggred_time = -1;
                eeprom.config_alarm_time[i].buffer_period_epoch = 120;
            }
        }
        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,1);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.alarm_volume = atoi(temp);
        }
        ESP_LOGI("alarm_volume", "%s,", temp);
        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10); 
        ESP_LOGI("time_mode", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            if(temp[0] == '0')
            {
                eeprom.time_mode = TIME_MODE_24HR;
            }
            else if(temp[0] == '1')
            {
                eeprom.time_mode = TIME_MODE_AM;
            }
        }
        cnt_temp++;
         memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10);
         ESP_LOGI("alarm_gap", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.alarm_gap =  (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.alarm_gap_unit =  G_SEC;unit = 1;}
            else if( temp[3] == 'M') {  eeprom.alarm_gap_unit =  G_MIN;unit = 60;}
            else if( temp[3] == 'H') {  eeprom.alarm_gap_unit =  G_HH; unit = 3600;}
            eeprom.alarm_gap_sec    = eeprom.alarm_gap*unit;
        }
        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10);
        ESP_LOGI("total alarm duration", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.TADE_interval =  (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.TADE_interval_unit =  G_SEC;unit = 1;}
            else if( temp[3] == 'M') {  eeprom.TADE_interval_unit =  G_MIN;unit = 60;}
            else if( temp[3] == 'H') {  eeprom.TADE_interval_unit =  G_HH; unit = 3600;}  
            eeprom.TADE_interval_sec = eeprom.TADE_interval*unit; 
        }
        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10);
        ESP_LOGI("panic_interval", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.panic_interval =  (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.panic_interval_unit =  G_SEC;unit= 1;}
            else if( temp[3] == 'M') {  eeprom.panic_interval_unit =  G_MIN;unit= 60;}
            else if( temp[3] == 'H') {  eeprom.panic_interval_unit =  G_HH; unit= 3600;} 
            eeprom.panic_interval_sec = eeprom.panic_interval*unit;  
        }

        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10);
        ESP_LOGI("ble_connection_timeout", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.ble_connection_timeout =  (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.ble_connection_timeout_unit =  G_SEC;unit= 1;}
            else if( temp[3] == 'M') {  eeprom.ble_connection_timeout_unit =  G_MIN;unit= 60;}
            else if( temp[3] == 'H') {  eeprom.ble_connection_timeout_unit =  G_HH; unit= 3600;} 
            eeprom.ble_connection_timeout_sec = eeprom.ble_connection_timeout*unit;  
        }
        ESP_LOGI(".", " eeprom.ble_connection_timeout %d",  eeprom.ble_connection_timeout);

        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10);
        ESP_LOGI("backlight_Off_timeout", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.backlight_Off_timeout =  (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.backlight_Off_timeout_unit =  G_SEC;unit= 1;}
            else if( temp[3] == 'M') {  eeprom.backlight_Off_timeout_unit =  G_MIN;unit= 60;}
            else if( temp[3] == 'H') {  eeprom.backlight_Off_timeout_unit =  G_HH; unit= 3600;} 
            eeprom.backlight_Off_timeout_sec = eeprom.backlight_Off_timeout*unit;  
        }
        ESP_LOGI(".", " eeprom.backlight_Off_timeout %d",  eeprom.backlight_Off_timeout);
        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10);
        ESP_LOGI("Display_timeout", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.display_timeout =  (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.display_timeout_unit =  G_SEC;unit= 1;}
            else if( temp[3] == 'M') {  eeprom.display_timeout_unit =  G_MIN;unit= 60;}
            else if( temp[3] == 'H') {  eeprom.display_timeout_unit =  G_HH; unit= 3600;} 
            eeprom.display_timeout_sec = eeprom.display_timeout*unit;  
        }
        ESP_LOGI(".", " eeprom.display_timeout %d == %ld",  eeprom.display_timeout, eeprom.display_timeout_sec);
        cnt_temp++;
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)data, ",",cnt_temp,0,0,0,10);
        ESP_LOGI("battery_alert_threshold", "%s,", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.battery_alert_threshold =  (temp[0] - '0') * 10 + (temp[1] - '0');
        }
        ESP_LOGI(".", " eeprom.battery_alert_threshold %d",  eeprom.battery_alert_threshold);
        if(flag != 2)
        {
            write_eeprom((char *)&eeprom);
            return(1);
        }
        return(0);
    }

//!   FTP,FILE_NAME,FILE_SIZE,TOTAL_PACKET_IN_FILE,FILE_CHECKSUM,FILE_CHECKXOR,PACKET_SIZE(CHUNK),PACKET_CHECKSUM,PACKET_CHECKXOR,&
//!   Example: FTP,1.wav,20000,40,38,25,500,52,13,&

    void Decode_Filename(char data, char *file_Name)
    {
        switch(data)
        {
            case 1:
                memcpy(file_Name,"/littlefs/1.txt",strlen("/littlefs/1.txt"));
			break;
            case 2:
                memcpy(file_Name,"/littlefs/2.txt",strlen("/littlefs/2.txt"));
			break;
            case 3:
                memcpy(file_Name,"/littlefs/3.txt",strlen("/littlefs/3.txt"));
			break;
            case 4:
                memcpy(file_Name,"/littlefs/4.txt",strlen("/littlefs/4.txt"));
			break;
            case 5:
                memcpy(file_Name,"/littlefs/5.txt",strlen("/littlefs/5.txt"));
			break;
            case 6:
                memcpy(file_Name,"/littlefs/6.txt",strlen("/littlefs/6.txt"));
			break;
            case 7:
                memcpy(file_Name,"/littlefs/7.txt",strlen("/littlefs/7.txt"));
			break;
            case 8:
                memcpy(file_Name,"/littlefs/8.txt",strlen("/littlefs/8.txt"));
            break;
            case 9:
                memcpy(file_Name,"/littlefs/9.txt",strlen("/littlefs/9.txt"));
            break;
            case 10:
                memcpy(file_Name,"/littlefs/10.txt",strlen("/littlefs/10.txt"));
            break;
            case 11:
                memcpy(file_Name,"/littlefs/11.txt",strlen("/littlefs/11.txt"));
            break;
            case 12:
                memcpy(file_Name,"/littlefs/12.txt",strlen("/littlefs/12.txt"));
            break;
            case 13:
                memcpy(file_Name,"/littlefs/13.txt",strlen("/littlefs/13.txt"));
            break;
            case 14:
                memcpy(file_Name,"/littlefs/14.txt",strlen("/littlefs/14.txt"));
            break;
            case 15:
                memcpy(file_Name,"/littlefs/15.txt",strlen("/littlefs/15.txt"));
            break;
            case 16:
                memcpy(file_Name,"/littlefs/16.txt",strlen("/littlefs/16.txt"));
            break;
            case 17:
                memcpy(file_Name,"/littlefs/17.txt",strlen("/littlefs/17.txt"));
            break;
            case 18:
                memcpy(file_Name,"/littlefs/18.txt",strlen("/littlefs/18.txt"));
            break;
            case 19:
                memcpy(file_Name,"/littlefs/19.txt",strlen("/littlefs/19.txt"));
            break;
            case 20:
                memcpy(file_Name,"/littlefs/20.txt",strlen("/littlefs/20.txt"));
            break;
            case 21:
                memcpy(file_Name,"/littlefs/21.txt",strlen("/littlefs/21.txt"));
            break;
            case 22:
                memcpy(file_Name,"/littlefs/22.txt",strlen("/littlefs/22.txt"));
            break;
            case 23:
                memcpy(file_Name,"/littlefs/23.txt",strlen("/littlefs/23.txt"));
            break;
            case 24:
                memcpy(file_Name,"/littlefs/24.txt",strlen("/littlefs/24.txt"));
            break;
            case 25:
                memcpy(file_Name,"/littlefs/25.txt",strlen("/littlefs/25.txt"));
            break;
            case 26:
                memcpy(file_Name,"/littlefs/26.txt",strlen("/littlefs/26.txt"));
            break;
            case 27:
                memcpy(file_Name,"/littlefs/27.txt",strlen("/littlefs/27.txt"));
            break;
            case 28:
                memcpy(file_Name,"/littlefs/28.txt",strlen("/littlefs/28.txt"));
            break;
            default:
                memcpy(file_Name,"/littlefs/OTA.txt",strlen("/littlefs/OTA.txt"));
            break;
        }			
    }
    
    static char  packet_checksum = 0, packet_xor = 0;
    void Decode_file_information_from_file(char *data)
    {
        char temp[20] = {0};
        memset((char*)&file_info, 0x00, sizeof(FileInfo));     
        // File name
        get_parameter_from_data((char*)&temp,(char*)data,",",1,"FTP",0,0,7);
        char FN = 0;    FN = atoi(temp);
        Decode_Filename(FN,(char*)&file_info.file_name);
        ESP_LOGI("BLE", "File name %s", file_info.file_name);
        // total File size
        get_parameter_from_data((char*)&temp,(char*)data,",",2,"FTP",0,0,8);
        file_info.file_size = atoi(temp);       memset((char*)&temp, 0x00, sizeof(temp));
        // Total number No OF packet in the file
        get_parameter_from_data((char*)&temp,(char*)data,",",3,"FTP",0,0,4);
        file_info.files_total_packet = atoi(temp); memset((char*)&temp, 0x00, sizeof(temp));
        // File Checksum
        get_parameter_from_data((char*)&temp,(char*)data,",",4,"FTP",0,0,4);
        file_info.file_checksum = atoi(temp);   memset((char*)&temp, 0x00, sizeof(temp));
        // File Check xor
        get_parameter_from_data((char*)&temp,(char*)data,",",5,"FTP",0,0,4);
        file_info.file_checkxor = atoi(temp);   memset((char*)&temp, 0x00, sizeof(temp));
        // Chunk size
        get_parameter_from_data((char*)&temp,(char*)data,",",6,"FTP",0,0,4);
        file_info.packet_data_size = atoi(temp);
        ESP_LOGI("file","chksum %x, chkxor %x",file_info.file_checksum, file_info.file_checkxor);
    }
    
    uint8_t Decode_first_packet_information() 
    {
        // 1. We checked the header and Footer of the string
        ESP_LOGI("BLE", "%x,====%s====  %c",Ble_rxbuff[Ble_length - 1], Ble_rxbuff, Ble_rxbuff[Ble_length - 1] );
        packet_checksum = 0; packet_xor = 0;
        if((Ble_rxbuff[0] == 'F')&&(Ble_rxbuff[1] == 'T')&&(Ble_rxbuff[2] == 'P')&&(Ble_rxbuff[Ble_length - 1] == '&'))
        {
            // 2. We calculate the packet checksum and checkxor
            for (int i = 0; i < (Ble_length - 3); i++)
            {
                packet_checksum += (unsigned char)Ble_rxbuff[i];
                packet_xor ^= (unsigned char)Ble_rxbuff[i];
            }
            // ESP_LOGI("INSIDE","chksum%x, chkxor%x",packet_checksum,packet_xor);

            // 3. Now we will matched the received packet checksum and check xor
            if(!((packet_checksum == Ble_rxbuff[Ble_length-3]) && (packet_xor == Ble_rxbuff[Ble_length-2])))
            {
                Send_BLEdata((char*)"ERROR:FPCHKSUMFAILED", 0xFFFF);
                memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                packet_checksum = 0; packet_xor = 0;
                return (0);
            }
            else       
            {
                // 4. If Checksum and Check xor matched then we will store all the information into RAM structure
                Decode_file_information_from_file((char*)&Ble_rxbuff);
                
                ESP_LOGI("BLE", "File name %s", file_info.file_name);
                Delete_File((char*)file_info.file_name); 
                
                // Save first packet in the memory to further use
                g_Ble_rxbuff_subsc = 0;
                // Now we will check the received file for OTA or Sound File
                if(strstr((char*)file_info.file_name, "/littlefs/OTA.txt") != NULL)
                {
                    return(2);
                }
                else
                {
                    WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&Ble_rxbuff,496);
                    g_Ble_rxbuff_subsc = 496;
                    return 1;
                }
            }
        }
        // ESP_LOGI("BLE", "EXIT IN THE FIRST PACKET DECODE INFORMATION");
        return 0;
    }

    void Handle_Sound_File()
    {
        ESP_LOGI("SOUND","Enter in the Handle Sound Function");
        uint32_t timeout = 0;
        uint16_t l_pkt_no = 0;
        char file_chksum = 0, file_xor = 0;
       // char bufff[50000];
        // g_Ble_rxbuff_subsc = 0;
        char* bufff = (char*) malloc(50000);//file_info.file_size + 500);
        // if(sound_data == NULL)
        // {
        //     Send_BLEdata((char*)"ERROR:MEMALLOCATION",0xFFFF);
        //     return;
        // }
        // memcpy((char*)&sound_data[g_Ble_rxbuff_subsc],(char *)&Ble_rxbuff[0],496);
        // g_Ble_rxbuff_subsc = 496;

       
        memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
        ESP_LOGI("SOUND 0"," g_Ble_rxbuff_subsc %ld", g_Ble_rxbuff_subsc);
        packet_checksum = 0; packet_xor = 0;Total_packet_recived = 0;
        Send_BLEdata((char*)"OK",0xFFFF);
        uint16_t pkt_len = 0,temp_len=0;
        while (1)
        {
            vTaskDelay(10/portTICK_PERIOD_MS);
            if((++timeout >= 60000) || (Is_ble_connected != 1))          // Timeout Decide 5 Min
            {
                timeout = 0;
                Send_BLEdata((char*)"ERROR:TIMEOUT",0xFFFF);
                memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                memset((char *)&file_info, 0x00, sizeof(FileInfo));
                g_Ble_rxbuff_subsc = 0; packet_checksum = 0; packet_xor = 0;
                PB_esp_restart();
            }
            if(f.g_ble_rcvd_F == 1)
            {
                f.g_ble_rcvd_F = 0;
                if((Ble_rxbuff[0] == 'F') && (Ble_rxbuff[1] == 'T'))            // include FT and Checksum checkm xor in packet data size
                {
                    if(file_info.files_total_packet == (l_pkt_no+1))
                    {
                        pkt_len = file_info.file_size - (l_pkt_no *(file_info.packet_data_size - 4));
                        pkt_len += 4;
                    }
                    else
                    {
                        pkt_len = file_info.packet_data_size;
                    }

                    if((Ble_length == pkt_len) )       // include FT and Checksum checkm xor in packet data size
                    {
                        packet_checksum=0;packet_xor=0;
                        for (int i = 0; i < (Ble_length - 2); i++)
                        {
                            packet_checksum += (unsigned char)Ble_rxbuff[i];
                            packet_xor ^= (unsigned char)Ble_rxbuff[i];
                        }
                        if(!((packet_checksum == Ble_rxbuff[pkt_len-2]) && (packet_xor == Ble_rxbuff[pkt_len-1])))
                        {
                            Send_BLEdata((char*)"ERROR:CHECKSUMMISMATCH",0xFFFF);
                            memset((char *)&file_info, 0x00, sizeof(FileInfo));
                            memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                            g_Ble_rxbuff_subsc = 0; packet_checksum = 0; packet_xor = 0;
                            // PB_esp_restart();
                        }
                        else
                        {
                            for (int j = 2; j < (Ble_length-2); j++)
                            {
                                file_chksum += (char)Ble_rxbuff[j];
                                file_xor ^=  (char)Ble_rxbuff[j];
                            }
                            memcpy((char*)&bufff[temp_len],(char *)&Ble_rxbuff[2],Ble_length-4);
                            //WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&Ble_rxbuff[2],Ble_length-4);
                            temp_len+=(Ble_length - 4);
                            // memcpy((char*)&sound_data[g_Ble_rxbuff_subsc],(char *)&Ble_rxbuff[2],Ble_length-4);
                            //g_Ble_rxbuff_subsc += (Ble_length - 4);
                            // ESP_LOGI("SOUND"," g_Ble_rxbuff_subsc %ld", g_Ble_rxbuff_subsc);
                            if(temp_len>=49000)
                            {
                                WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&bufff[0],temp_len);
                                g_Ble_rxbuff_subsc += temp_len;
                                temp_len=0;
                            }
                            l_pkt_no++;
                            Send_BLEdata((char*)"OK",0xFFFF);
                            if(file_info.files_total_packet == l_pkt_no)
                            {
                                 if(temp_len!=0)
                                {
                                    WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&bufff[0],temp_len);
                                    g_Ble_rxbuff_subsc += temp_len;
                                    temp_len=0;
                                }
                                //ESP_LOGI("FILECHECKSUMM","file_info.file_checksum %2X file_chksum %2X, file_info.file_checkxor %2X file_xor %2X",file_info.file_checksum,file_chksum,file_info.file_checkxor,file_xor);
                                if((file_info.file_checksum == file_chksum) && (file_info.file_checkxor == file_xor))
                                {
                                    // int written_bytes = 0, written_count = 1024;
                                    // while(1)
                                    // {
                                    //     if(g_Ble_rxbuff_subsc < 1024)
                                    //     {
                                    //         written_count = g_Ble_rxbuff_subsc;
                                    //     }
                                    //     if(g_Ble_rxbuff_subsc == 0)
                                     //     {
                                    //         break;
                                    //     }
                                        // WriteDataToFS((char*)file_info.file_name,0,(char *)&sound_data[0],g_Ble_rxbuff_subsc);
                                    //     ESP_LOGI("SOUND"," g_Ble_rxbuff_subsc %d, written_count %d,written_bytes %d ", g_Ble_rxbuff_subsc, written_count, written_bytes);
                                    //     g_Ble_rxbuff_subsc -= g_Ble_rxbuff_subsc;
                                    //     written_bytes += g_Ble_rxbuff_subsc;
                                    // }
                                     free(bufff);
                                    //WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&Ble_rxbuff[2],Ble_length-4);
                                    Send_BLEdata((char*)"FILE RECEIVED SUCCESSFULLY",0xFFFF);      
                                    memset((char*)&file_info, 0x00, sizeof(FileInfo));
                                    return; 
                                }
                                else
                                {
                                    // WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&Ble_rxbuff[2],Ble_length-4,CLOSE);
                                    Send_BLEdata((char*)"ERROR:FILECHECKSUMMISMATCH",0xFFFF);
                                    memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                                    g_Ble_rxbuff_subsc = 0; packet_checksum = 0; packet_xor = 0;
                                    memset((char *)&file_info, 0x00, sizeof(FileInfo));
                                    // PB_esp_restart();
                                }
                            }
                        }
                    }
                    else 
                    {
                        // WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&Ble_rxbuff[2],Ble_length-4,CLOSE);
                        Send_BLEdata((char*)"ERROR:PACKETLENGTHMISMATCH",0xFFFF);
                        memset((char *)&file_info, 0x00, sizeof(FileInfo));
                        // PB_esp_restart();

                    }
                }
                else
                {
                    // WriteDataToFS((char*)file_info.file_name,g_Ble_rxbuff_subsc,(char *)&Ble_rxbuff[2],Ble_length-4,CLOSE);
                    Send_BLEdata((char*)"ERROR:HEADERMISMATCH",0xFFFF);
                    memset((char *)&file_info, 0x00, sizeof(FileInfo));
                    memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                    // PB_esp_restart(); 
                }
            }
        }
        free(bufff);
    }

    void Handle_FWM_upgrade()
    {
        uint32_t timeout = 0;
        uint16_t l_pkt_no = 0;
        char file_chksum = 0, file_xor = 0;
        ota_init_setup();
        Send_BLEdata((char*)"OK",0xFFFF);
        uint16_t pkt_len = 0;
        while (1)
        {
            vTaskDelay(10/portTICK_PERIOD_MS);
            if((++timeout >= 60000) || (Is_ble_connected != 1))          // Timeout Decide 5 Min
            {
                timeout = 0;
                Send_BLEdata((char*)"ERROR:TIMEOUT",0xFFFF);
                memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                memset((char *)&file_info, 0x00, sizeof(FileInfo));
                g_Ble_rxbuff_subsc = 0; packet_checksum = 0; packet_xor = 0;
                PB_esp_restart();
            }
            if(f.g_ble_rcvd_F == 1)
            {
                f.g_ble_rcvd_F = 0;
                if((Ble_rxbuff[0] == 'F') && (Ble_rxbuff[1] == 'T'))            // include FT and Checksum checkm xor in packet data size
                {
                    if(file_info.files_total_packet == (l_pkt_no+1))
                    {
                        pkt_len = file_info.file_size - (l_pkt_no *(file_info.packet_data_size - 4));
                        pkt_len += 4;
                    }
                    else
                    {
                        pkt_len = file_info.packet_data_size;
                    }
                    if((Ble_length == pkt_len) )       // include FT and Checksum checkm xor in packet data size
                    {
                        packet_checksum=0;packet_xor=0;
                        for (int i = 0; i < (Ble_length - 2); i++)
                        {
                            packet_checksum += (unsigned char)Ble_rxbuff[i];
                            packet_xor ^= (unsigned char)Ble_rxbuff[i];
                        }
                        if(!((packet_checksum == Ble_rxbuff[pkt_len-2]) && (packet_xor == Ble_rxbuff[pkt_len-1])))
                        {
                            Send_BLEdata((char*)"ERROR:CHECKSUMMISMATCH",0xFFFF);
                            memset((char *)&file_info, 0x00, sizeof(FileInfo));
                            memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                            g_Ble_rxbuff_subsc = 0; packet_checksum = 0; packet_xor = 0;
                            // PB_esp_restart();
                        }
                        else
                        {
                            for (int j = 2; j < (Ble_length-2); j++)
                            {
                                file_chksum += (char)Ble_rxbuff[j];
                                file_xor ^=  (char)Ble_rxbuff[j];
                            }
                            ota_data_write((char *)&Ble_rxbuff[2],Ble_length-4);
                            l_pkt_no++;
                            Send_BLEdata((char*)"OK",0xFFFF);
                            if(file_info.files_total_packet == l_pkt_no)
                            {
                                //ESP_LOGI("FILECHECKSUMM","file_info.file_checksum %2X file_chksum %2X, file_info.file_checkxor %2X file_xor %2X",file_info.file_checksum,file_chksum,file_info.file_checkxor,file_xor);
                                if((file_info.file_checksum == file_chksum) && (file_info.file_checkxor == file_xor))
                                {
                                    Send_BLEdata((char*)"FILE RECEIVED SUCCESSFULLY",0xFFFF);
                                    if(ota_final_setup())
                                    {
                                        Send_BLEdata("Firmware Upgrade Successfully", sizeof("Firmware Upgrade Successfully"));
                                        PB_esp_restart();
                                    }
                                    else
                                    {
                                        Send_BLEdata("Firmware Upgrade Failed", sizeof("Firmware Upgrade Failed"));
                                    }
                                    memset((char*)&file_info, 0x00, sizeof(FileInfo));
                                    return; 
                                }
                                else
                                {
                                    Send_BLEdata((char*)"ERROR:FILECHECKSUMMISMATCH",0xFFFF);
                                    memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                                    g_Ble_rxbuff_subsc = 0; packet_checksum = 0; packet_xor = 0;
                                    memset((char *)&file_info, 0x00, sizeof(FileInfo));
                                    // PB_esp_restart();
                                }
                            }
                        }
                    }
                    else 
                    {
                        Send_BLEdata((char*)"ERROR:PACKETLENGTHMISMATCH",0xFFFF);
                        memset((char *)&file_info, 0x00, sizeof(FileInfo));
                        // PB_esp_restart();

                    }
                }
                else
                {
                    Send_BLEdata((char*)"ERROR:HEADERMISMATCH",0xFFFF);
                    memset((char *)&file_info, 0x00, sizeof(FileInfo));
                    memset((char *)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                    // PB_esp_restart(); 
                }
            }
        }   
    }
    
    void Handle_receive_file()
    {
        uint8_t ret = Decode_first_packet_information();
        if(ret == 1)
        {
            Handle_Sound_File();
        }
        else if(ret == 2)
        {
            Handle_FWM_upgrade();
        }
            
    }
   
   char data_collection_buffer[2000]={0};
   
   void Handle_BLE_Data()
   {
        if(f.g_ble_rcvd_F == 1)
        {
            // Reset flags and counters when BLE data is received
            f.g_ble_rcvd_F = 0; 
            f.g_reset_counter_and_flag_F = 1;
            
            printf(" BLE RECEIVED DATA : %s \r\n", Ble_rxbuff);

            // Handle configuration data packets starting with $$
            if(strstr(Ble_rxbuff,"$$"))
            {
                // Clear buffer if new configuration sequence starts with $$0
                if(strstr(Ble_rxbuff,"$$0"))
                {
                    memset((char*)&data_collection_buffer, 0x00, sizeof(data_collection_buffer));
                }
                
                // Append configuration data after $$ prefix
                strcat((char*)&data_collection_buffer,(char*)&Ble_rxbuff[4]);
                
                // Check if this is end of configuration sequence
                char* l_ptr = strstr(Ble_rxbuff,",%%");
                if(l_ptr!=0)
                {
                    *l_ptr=0;
                    handle_configuration_pkt((char*)&data_collection_buffer);
                }
                else{
                    Send_BLEdata("Ok", strlen("Ok"));
                }
                
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
            }

            // Device identification request
            else if(strstr(Ble_rxbuff,"Hey?"))
            {
                Send_BLEdata("This is Pill-Box", strlen("This is Pill-Box"));
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
            }

            // Factory reset command
            else if(strstr(Ble_rxbuff,"FR"))
            {
                Send_BLEdata("Flash Erase Complete", strlen("Flash Erase Complete"));
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                memset((char*)&eeprom, 0x00, sizeof(eeprom));
                write_eeprom((char *)&eeprom);
                PB_esp_restart();
            }

            // Configure device polarity for PCBA
            else if(strstr(Ble_rxbuff,"PCBA_POLARITY"))
            {
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                Send_BLEdata("Device configured for PCBA Polarity", strlen("Device configured for PCBA Polarity"));
                eeprom.Device_MGNT_Polar = PCBA_POLAR;
                write_eeprom((char *)&eeprom);
                PB_esp_restart();
            }

            // Configure device polarity for cabinet
            else if(strstr(Ble_rxbuff,"CAB_POLARITY"))
            {
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
                Send_BLEdata("Device configured for CABINET Polarity", strlen("Device configured for CABINET Polarity"));
                eeprom.Device_MGNT_Polar = CABINET_POLAR;
                write_eeprom((char *)&eeprom);
                PB_esp_restart();
            }

            // Get device model
            else if(strstr(Ble_rxbuff,"MODEL::?"))
            {
                Send_BLEdata(MODEL, sizeof(MODEL));
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
            }

            // Get firmware version
            else if(strstr(Ble_rxbuff,"VERSION?"))
            {
                char temp[20] = {0};
                sprintf(temp,"%s", FIRMWARE_VERSION);
                Send_BLEdata(temp, strlen(temp));
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
            }
            else if(strstr(Ble_rxbuff,"PRECFG::?"))    //  PRECFG,OK_TOLERENCE,PROLONG_THRESHOLD,WARNING_THRESHOLD,NO_OF_TIME_VOICEPLAY,&
            {
                fatch_preset_configure_data((char*)&data_collection_buffer);                
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
            }
            
            else if(strstr(Ble_rxbuff,"PRECFG,"))
            {
                if(handle_presetconfigure_configuration_pkt((char*)&Ble_rxbuff))
                {
                    Send_BLEdata("Device Preset Configured successfully", sizeof("Device Preset Configured successfully"));
                }
                else
                {
                    Send_BLEdata("Device Preset Configured Failed", sizeof("Device Preset Configured Failed"));
                }
            }

            // Get current configuration
            else if(strstr(Ble_rxbuff,"CFG::?"))
            {
                fatch_data((char*)&data_collection_buffer);                
                memset((char*)&Ble_rxbuff, 0x00, sizeof(Ble_rxbuff));
            }
            // Handle configuration command
            else if(strstr(Ble_rxbuff,"CFG,"))
            {
                handle_configuration_pkt((char*)&Ble_rxbuff);
            }
            else if(strstr(Ble_rxbuff,"TEST_DISCONNECT"))
            {
                ble_disconnect_connected_device();
            }
            // Get BLE advertisement data
            else if(strstr(Ble_rxbuff,"Fetch_Ble_Adv?"))
            {
                ESP_LOGI("PB DATA", "BATTERY %d", PB_reading.battery_percentage);
                Send_BLEdata((char*)&PB_reading, sizeof(PB_Data));
            }

            // Play audio command
            else if(strstr(Ble_rxbuff,"PLAYAUDIO"))
            {
                Start_play_sound();
            }

            // Handle file transfer
            else if(strstr(Ble_rxbuff,"FTP,"))
            {
                Handle_receive_file();
            }

            // Erase pill box report data
            else if(strstr(Ble_rxbuff,"PB_REPORT_ERASE"))
            {
                if(Delete_memory_data())
                {
                    Send_BLEdata("ERASED SUCCESSFULLY", sizeof("ERASED SUCCESSFULLY"));
                }
            }

            // Fetch pill box report data
            else if(strstr(Ble_rxbuff,"PB_REPORT_FETCH"))
            {
                char temp[120]={0};
                
                // Send timer configuration data for each timer
                for(int i = 0; i < eeprom.total_timers_configured; i++)
                {
                    memset((char*)&temp[0],0x00,sizeof(temp));
                    uint8_t Box_location = eeprom.config_alarm_time[i].Box_ID;
                    sprintf(temp,"T%03d,%02d,%d,%ld,%ld,%d",
                        i,
                        Box_location,
                        eeprom.config_alarm_time[i].ID_CONFIGURED_STATUS,
                        eeprom.config_alarm_time[i].timer_epoch,
                        eeprom.config_alarm_time[i].timer_result_epoch,
                        eeprom.config_alarm_time[i].alarm_repetition_cnt);
                    Send_BLEdata(temp, strlen(temp));
                }

                // Send wrong action data for each box
                for(int i = 0; i < MAXIMUM_SUPPORTED_BOXES; i++)
                {
                    memset((char*)&temp[0],0x00,sizeof(temp));
                    sprintf(temp,"$%02d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,&",
                        i,
                        eeprom.wrong_action[i].box_cnt,
                        eeprom.wrong_action[i].epoch[0],
                        eeprom.wrong_action[i].epoch[1], 
                        eeprom.wrong_action[i].epoch[2],
                        eeprom.wrong_action[i].epoch[3],
                        eeprom.wrong_action[i].epoch[4],
                        eeprom.wrong_action[i].epoch[5],
                        eeprom.wrong_action[i].epoch[6],
                        eeprom.wrong_action[i].epoch[7],
                        eeprom.wrong_action[i].epoch[8],
                        eeprom.wrong_action[i].epoch[9]);
                    Send_BLEdata(temp, strlen(temp));
                }               
                
                
                // Send Prolong data for each box
                for(int i = 0; i < MAXIMUM_SUPPORTED_BOXES; i++)
                {
                    memset((char*)&temp[0],0x00,sizeof(temp));
                    sprintf(temp,"$P%02d,%d,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld,&",
                        i,
                        eeprom.prolong_action[i].prolong_cnt,
                        eeprom.prolong_action[i].prolonged_epoch[0],
                        eeprom.prolong_action[i].prolonged_epoch[1],
                        eeprom.prolong_action[i].prolonged_epoch[2],
                        eeprom.prolong_action[i].prolonged_epoch[3],
                        eeprom.prolong_action[i].prolonged_epoch[4],
                        eeprom.prolong_action[i].prolonged_epoch[5],
                        eeprom.prolong_action[i].prolonged_epoch[6],
                        eeprom.prolong_action[i].prolonged_epoch[7],
                        eeprom.prolong_action[i].prolonged_epoch[8],
                        eeprom.prolong_action[i].prolonged_epoch[9]);
                    Send_BLEdata(temp, strlen(temp));
                }               
            }

            // Handle invalid requests
            else
            {
                uint16_t descr_value = Ble_rxbuff[1]<<8 | Ble_rxbuff[0]; 
                if(descr_value == 0x0001) {return;}
                Send_BLEdata("Invalid Request", sizeof("Invalid Request"));
            }
        }
   }
    
    void handle_configuration_pkt(char* buff)
    {
        ESP_LOGI("buff", "%s, len %d", buff, strlen(buff));
        char checksum = 0,calculated_chksum = 0;
        char checksum_get[3] = {0};
        char* ptr;
        get_parameter_from_data((char*)&checksum_get,(char*)buff, "*",1,0,0,0,2);
        checksum = (checksum_get[0] - '0')*10 + (checksum_get[1]-'0');
        ptr = strstr((char*)buff,"*");*(ptr+1) = 0;
        calculated_chksum = calc_chksum((char*)buff,strlen((char*)buff));
        ESP_LOGI("BLE", "CHECKSUM Received %d,CHECKSUM calculated %d", checksum,calculated_chksum);
        // if(checksum == calculated_chksum)           //!!!  ADD BASED OF COMMAS, VALIDATION ALGO
        if(1)
        {
            if(Decode_save_data((char*)buff))
            {
                Send_BLEdata("Device Configured successfully", sizeof("Device Configured successfully"));
            }
            else
            {
                Send_BLEdata("Device Configured Failed", sizeof("Device Configured Failed"));
            }
        }
        else
        {
            Send_BLEdata("Device Configured Failed", sizeof("Device Configured Failed"));
        }
        
    }
    
    void Bluetooth_data_advertisement()
    {
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret){
            printf( "config adv data failed, error code = %x\n", ret);
        }
    }

    void Bluetooth_deinit()
    {
        if(Is_ble_init == true)
        {
            Is_ble_init = false;
            esp_bluedroid_disable();
            esp_bluedroid_deinit();
            esp_bt_controller_disable();
            esp_bt_controller_deinit();
            
        }
    }

    void Bluetooth_init()
    {
        uint8_t mac[7]={0};
        esp_read_mac(mac,ESP_MAC_BT);
        ESP_LOGI("BLE MAC"," %s", mac);
        memcpy((char*)PB_reading.mac,(char*)mac,6);
        memcpy((char*)PB_reading.deviceID,(char*)MODEL,4);
        PB_reading.ALERT_TYPE = ALERT_TYPE;
        PB_reading.ALERT_INFO1 = ALERT_INFO1;
        PB_reading.ALERT_INFO2 = ALERT_INFO2;
        // sprintf((char*)&test_manufacturer,"%x%x%x%x%x%x%x",TEMP1, TEMP2, HUMD, BAT_PER,ALERT_TYPE,ALERT_INFO1,ALERT_INFO2);
        //ESP_LOGI("BLE", "BLUETOOTH INIT,  %s",PB_reading.device_version);
        if(Is_ble_init == false)
        {
            Is_ble_init = true;
            ESP_LOGI("Device name: ", "Device Name:%s, Device Pass:%d, Device Model <<%s>>", eeprom.device_name, eeprom.BLE_PASS, PB_reading.deviceID);
            esp_err_t ret;
            ret = nvs_flash_init();                             // Initialize NVS
            if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
            }
            ESP_ERROR_CHECK( ret );
            
            esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
            esp_bt_controller_init(&bt_cfg);
            esp_bt_controller_enable(ESP_BT_MODE_BLE);
            esp_bluedroid_init();
            esp_bluedroid_enable();
            esp_ble_gatts_register_callback(gatts_event_handler);
            esp_ble_gap_register_callback(gap_event_handler);
            esp_ble_gatts_app_register(PROFILE_A_APP_ID);
            esp_ble_gatt_set_local_mtu(517);

        

            // ESP_LE_AUTH_NO_BOND          : No bonding.
            // ESP_LE_AUTH_BOND             : Bonding is performed.
            // ESP_LE_AUTH_REQ_MITM         : MITM Protection is enabled.
            // ESP_LE_AUTH_REQ_SC_ONLY      : Secure Connections without bonding enabled.
            // ESP_LE_AUTH_REQ_SC_BOND      : Secure Connections with bonding enabled.
            // ESP_LE_AUTH_REQ_SC_MITM      : Secure Connections with MITM Protection and no bonding enabled.
            // ESP_LE_AUTH_REQ_SC_MITM_BOND : Secure Connections with MITM Protection and bonding enabled.

            esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;                 //bonding with peer device after authentication

          

            esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;                                    //set the IO capability to No output No input
            uint8_t key_size = 16;                                                      //the key size should be 7~16 bytes
            uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
            uint8_t rsp_key  = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
            uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_ENABLE;
            esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &eeprom.BLE_PASS, sizeof(uint32_t));
            esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
            esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
            esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
            esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
            esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
            esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

        }
    }

    /**
     * @brief Handle preset configuration packet
     * 
     * This function decodes and saves the preset configuration data received via BLE.
     * The data includes:
     * - OK tolerance
     * - Delay tolerance
     * - Prolong threshold
     * - Warning threshold
     * - Number of times voice play
     *
     * @param pbuffer Pointer to the buffer containing the preset configuration data
    */

    char handle_presetconfigure_configuration_pkt(char *pbuffer)
    {
        //PRECFG,OK_TOLERENCE,PROLONG_THRESHOLD,WARNING_THRESHOLD,NO_OF_TIME_VOICEPLAY,&
        char temp[20] = {0};
        int unit = 0;
        ESP_LOGI("PRECFG", "RECVD DATA: %s, len %d", pbuffer, strlen(pbuffer));
        
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)pbuffer, ",",1,0,0,0,1);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.pre_cfg.temperature_unit = temp[0] - '0';
        }
        ESP_LOGI("=", "%s === eeprom.pre_cfg.temperature_unit %d", temp, eeprom.pre_cfg.temperature_unit);
       
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)pbuffer, ",",2,0,0,0,4);
        ESP_LOGI("=", "%s", temp);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.pre_cfg.prolong_threshold = (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.pre_cfg.prolong_threshold_unit =  G_SEC;     unit =1;}
            else if( temp[3] == 'M') {  eeprom.pre_cfg.prolong_threshold_unit =  G_MIN;   unit = 60;}
            else if( temp[3] == 'H') {  eeprom.pre_cfg.prolong_threshold_unit =  G_HH;  unit = 3600;} 

            eeprom.pre_cfg.prolong_threshold_sec = eeprom.pre_cfg.prolong_threshold*unit;
        }

        ESP_LOGI("=", "%s", temp);
        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)pbuffer, ",",3,0,0,0,4);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.pre_cfg.warning_threshold = (temp[0] - '0') * 100 + (temp[1] - '0') * 10 + (temp[2] - '0');
            if( temp[3] == 'S')      {  eeprom.pre_cfg.warning_threshold_unit =  G_SEC;     unit =1;}
            else if( temp[3] == 'M') {  eeprom.pre_cfg.warning_threshold_unit =  G_MIN;   unit = 60;}
            else if( temp[3] == 'H') {  eeprom.pre_cfg.warning_threshold_unit =  G_HH;  unit = 3600;} 

            eeprom.pre_cfg.warning_threshold_sec = eeprom.pre_cfg.warning_threshold*unit;
        }
        ESP_LOGI("=", "%s", temp);

        memset(temp, 0x00, sizeof(temp)); get_parameter_from_data((char*)&temp,(char*)pbuffer, ",",4,0,0,0,1);
        if(!((temp[0] == '-') && (temp[1] == '1')))
        {
            eeprom.pre_cfg.no_of_time_voiceplay = temp[0] - '0';
        }
        ESP_LOGI("=", "%s", temp);
        write_eeprom((char *)&eeprom);
        return(true);
    }

    void fatch_preset_configure_data(char *pbuffer)
    {
        //PRECFG,OK_TOLERENCE,PROLONG_THRESHOLD,WARNING_THRESHOLD,NO_OF_TIME_VOICEPLAY,&
        char temp[20] = {0};

        sprintf(pbuffer,"PRECFG,%d,",eeprom.pre_cfg.temperature_unit);
        
        sprintf(temp,"%03d",eeprom.pre_cfg.prolong_threshold); strcat(pbuffer,temp);
        if(eeprom.pre_cfg.prolong_threshold_unit == G_SEC)      {  strcat(pbuffer,"S,");}
        else if(eeprom.pre_cfg.prolong_threshold_unit == G_MIN) {  strcat(pbuffer,"M,");}
        else if(eeprom.pre_cfg.prolong_threshold_unit == G_HH)  {  strcat(pbuffer,"H,");}

        sprintf(temp,"%03d",eeprom.pre_cfg.warning_threshold); strcat(pbuffer,temp);
        if(eeprom.pre_cfg.warning_threshold_unit == G_SEC)      {  strcat(pbuffer,"S,");}
        else if(eeprom.pre_cfg.warning_threshold_unit == G_MIN) {  strcat(pbuffer,"M,");}
        else if(eeprom.pre_cfg.warning_threshold_unit == G_HH)  {  strcat(pbuffer,"H,");}

        sprintf(temp,"%d,",eeprom.pre_cfg.no_of_time_voiceplay);    
        strcat(pbuffer,temp);       strcat(pbuffer,"&");

        ESP_LOGI("PRECFG", "PRECFG DATA: %s, len %d", pbuffer, strlen(pbuffer));
        Send_BLEdata((char*)pbuffer, 0xFFFF);
    }

#endif