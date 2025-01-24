#include "header.h"

volatile unsigned int motion_5s_pulse_cnts = 0;

// Helper function to add microsecond delays
void delay_us(int time) {
    volatile unsigned int i;
    for(i = 0; i < 100; i++); // Simple delay loop
}

// Write data to accelerometer register
char acc_write(char skip, char device_add, char reg_add, char data) {
    char g_ack = 1;
    stop();
    delay_us(0);
    
    start(); // Start I2C communication
    g_ack = tx(device_add); // Select device for write
    delay_us(0);
    g_ack = tx(reg_add); // Select register
    delay_us(0); 
    g_ack = tx(data); // Send data
    stop(); // Stop I2C communication
    delay_us(0);
    
    return g_ack;
}

// Read data from accelerometer register 
char acc_read(char skip, char device_add, char reg_add) {
    char device_add_read = device_add | 0x01;
    char rdata = 0;

    start(); // Start I2C communication
    tx(device_add); // Select device for write
    tx(reg_add); // Select register
    start(); // Restart communication
    tx(device_add_read); // Select device for read
    rdata = rx(); // Read data
    stop();
    
    return rdata;
}

// Transmit byte over I2C
char tx(char x) {
    char i, ack;
    unsigned char mask = 0x80;
    
    SCL_OUTPUT;
    SDA_OUTPUT;
    delay_us(0);

    // Send 8 bits
    for(i = 0; i < 8; i++) {
        delay_us(0);
        if((x & mask)) {
            ACC_SDASET;
        } else {
            ACC_SDACLR;
        }
        delay_us(0);
        SCLSET;
        delay_us(0);
        SCLCLR;
        mask >>= 1;
    }

    // Check for ACK
    ACC_SDASET;
    delay_us(1);
    SDA_INPUT;
    delay_us(0);
    SCLSET;
    delay_us(0);
    
    ack = gpio_get_level(ACC_SDA) ? 1 : 0;
    
    SCLCLR;
    ACC_SDACLR;
    return ack;
}

// Generate I2C start condition
void start(void) {
    SCL_OUTPUT;
    SDA_OUTPUT;
    delay_us(0);
    ACC_SDASET;
    delay_us(0);
    SCLSET;
    delay_us(0);
    ACC_SDACLR;
    delay_us(0);
    SCLCLR;
}

// Generate I2C stop condition 
void stop(void) {
    SCL_OUTPUT;
    SDA_OUTPUT;
    delay_us(0);
    ACC_SDACLR;
    delay_us(0);
    SCLSET;
    delay_us(0);
    ACC_SDASET;
}

// Receive byte over I2C
char rx() {
    unsigned char d = 0, i;
    
    SCL_OUTPUT;
    SDA_INPUT;
    vTaskDelay(100/portTICK_PERIOD_MS);
    delay_us(0);

    // Read 8 bits
    for(i = 0; i < 8; i++) {
        SCLCLR;
        delay_us(0);
        SCLSET;
        delay_us(0);
        
        d = d << 1;
        if(gpio_get_level(ACC_SDA)) {
            d |= 0x01;
        }
        delay_us(0);
    }

    // Send ACK
    SCLCLR;
    delay_us(0);
    ACC_SDACLR;
    delay_us(0);
    SCLSET;
    delay_us(0);
    SCLCLR;
    ACC_SDACLR;
    delay_us(0);
    
    return d;
}

// Initialize accelerometer
void accelerometer_init(void) {
    volatile char who_am_i_reg_read = 0x00;
    printf("Init accelerometer\n");

    // Configure interrupt pin
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << ACC_INT1);
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(ACC_INT1, acc_isr_handler, (void *)ACC_INT1);
    
    vTaskDelay(10/portTICK_PERIOD_MS);
    stop();

    // Try to read WHO_AM_I register up to 5 times
    for(int i = 0; i < 5; i++) {
        vTaskDelay(100/portTICK_PERIOD_MS);
        who_am_i_reg_read = acc_read(0, ACC_ADDR, 0x0F);
        printf("\n check for accelerometer who_am_i_reg_read = %c,%x\r\n", 
               who_am_i_reg_read, who_am_i_reg_read);
        if(who_am_i_reg_read == 0x33) {
            break;
        }
    }

    // Configure accelerometer based on device type
    if(who_am_i_reg_read == 0x33) {
        // LIS3DH configuration
        acc_write(0, ACC_ADDR, CTRL_REG1, 0x77);
        acc_write(0, ACC_ADDR, CTRL_REG2, 0x09);
        acc_write(0, ACC_ADDR, CTRL_REG3, 0x40);
        acc_write(0, ACC_ADDR, CTRL_REG4, 0x00);
        acc_write(0, ACC_ADDR, CTRL_REG5, 0x00);
        acc_write(0, ACC_ADDR, CTRL_REG6, 0x02);
        acc_write(0, ACC_ADDR, INT1_THS, 0X10);
        acc_write(0, ACC_ADDR, INT1_DURATION, 0x00);
        acc_read(0, ACC_ADDR, REFERENCE);
        acc_write(0, ACC_ADDR, INT1_CFG, 0x2a);
        acc_read(0, ACC_ADDR, INT1_SOURCE);
    }
    else {
        who_am_i_reg_read = acc_read(0, 0xD6, 0x0F);
        
        if(who_am_i_reg_read == 0x6A) {
            // LSM6DS3 configuration
            acc_write(0, 0xD6, 0x10, 0x60);
            acc_write(0, 0xD6, 0x58, 0x8C);
            acc_write(0, 0xD6, 0x59, 0X01);
            acc_write(0, 0xD6, 0x5A, 0x06);
            acc_write(0, 0xD6, 0x5B, 0x00);
            acc_write(0, 0xD6, 0x17, 0x01);
            acc_write(0, 0xD6, 0x5C, 0x00);
            acc_write(0, 0xD6, 0x5F, 0x44);
            acc_write(0, 0xD6, 0x19, 0x0C);
            acc_write(0, 0xD6, 0x5E, 0x02);
        }
        else {
            // MMA8452Q configuration
            acc_write(0, 0x98, 0x07, 0x0b);
            acc_write(0, 0x98, 0x08, 0x15);
            acc_write(0, 0x98, 0x20, 0x35);
            acc_write(0, 0x98, 0x33, 0x0c);
            acc_write(0, 0x98, 0x43, 0x50);
            acc_write(0, 0x98, 0x44, 0x00);
            acc_write(0, 0x98, 0x45, 0x00);
            acc_write(0, 0x98, 0x09, 0x04);
            acc_write(0, 0x98, 0x4a, 0x00);
            acc_write(0, 0x98, 0x06, 0x44);
            acc_write(0, 0x98, 0x07, 0x01);
            acc_write(0, 0x98, 0x14, 0x00);
        }
    }
}

// Interrupt handler for accelerometer
void IRAM_ATTR acc_isr_handler(void* arg) {
    gpio_intr_disable((uint32_t) arg);
    f.g_motionsense_F = 1;
    gpio_intr_enable((uint32_t) arg);
}
