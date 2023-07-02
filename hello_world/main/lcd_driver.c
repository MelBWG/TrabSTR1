#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdio.h>
#include "esp_log.h"
#include "sdkconfig.h"
#include "unistd.h"


#define SLAVE_ADDRESS_LCD 0x4E>>1
#define SDA_PIN  21
#define SCL_PIN  22
#define LCD_COLS 16
#define LCD_ROWS 2

extern SemaphoreHandle_t SemaforoTemp;
extern SemaphoreHandle_t SemaforoUmid;
extern SemaphoreHandle_t SemaforoLumi;
extern uint8_t flag_init;
extern uint8_t scram_active;
extern float temp_value;
extern float umid_value;
extern float power_value;

esp_err_t err;
char buffer[17];

#define I2C_NUM I2C_NUM_0

static const char *TAG = "LCD";


static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_NUM_0;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}


void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4]; // Escrita em 4 bytes
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
	if (err!=0) ESP_LOGI(TAG, "Error in sending command");
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=0
	data_t[1] = data_u|0x09;  //en=0, rs=0
	data_t[2] = data_l|0x0D;  //en=1, rs=0
	data_t[3] = data_l|0x09;  //en=0, rs=0
	err = i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000);
	if (err!=0) ESP_LOGI(TAG, "Error in sending data");
}

void lcd_clear (void)
{
	lcd_send_cmd (0x01);
	usleep(5000);
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }

    lcd_send_cmd (col);
}


void lcd_init (void)
{
	// 4 bit initialisation
	usleep(50000);  // wait for >40ms
	lcd_send_cmd (0x30);
	usleep(5000);  // wait for >4.1ms
	lcd_send_cmd (0x30);
	usleep(200);  // wait for >100us
	lcd_send_cmd (0x30);
	usleep(10000);
	lcd_send_cmd (0x20);  // 4bit mode
	usleep(10000);

  // dislay initialisation
	lcd_send_cmd (0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	usleep(1000);
	lcd_send_cmd (0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	usleep(1000);
	lcd_send_cmd (0x01);  // clear display
	usleep(1000);
	usleep(1000);
	lcd_send_cmd (0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	usleep(1000);
	lcd_send_cmd (0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	usleep(1000);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}

/**
 * @brief i2c master initialization
 */



void taskLCD(void* param)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    lcd_init();
    lcd_clear();

    sprintf(buffer, "Temp:    Um:    ");
    lcd_put_cur(0, 0);
    lcd_send_string(buffer);
    sprintf(buffer, "Pot:     S:     ");
    lcd_put_cur(1, 0);
    lcd_send_string(buffer);
    char temp_buffer[3];
    char umid_buffer[3];
    char luz_buffer[4];

    while (true) {
        if(flag_init) {
            if(xSemaphoreTake(SemaforoTemp, (TickType_t) 100)) {
                sprintf(temp_buffer, "%2d", (int) temp_value);
                lcd_put_cur(0, 6);
                lcd_send_string(temp_buffer);
                xSemaphoreGive(SemaforoTemp);
            }
            if(xSemaphoreTake(SemaforoUmid, (TickType_t) 100)) {
                sprintf(umid_buffer, "%2d", (int) umid_value);
                lcd_put_cur(0, 13);
                lcd_send_string(umid_buffer);
                xSemaphoreGive(SemaforoUmid);
            }
            if(xSemaphoreTake(SemaforoLumi, (TickType_t) 100)) {
                sprintf(luz_buffer, "%3d", (int) power_value);
                lcd_put_cur(1, 4);
                lcd_send_string(luz_buffer);
                xSemaphoreGive(SemaforoLumi);
            }
            if(!scram_active) {
                lcd_put_cur(1, 13);
                lcd_send_string("OK");
            }
            else {
                lcd_put_cur(1, 11);
                lcd_send_string("SCRAM");
            }
        }
        else if(!scram_active) {
            lcd_put_cur(1, 12);
            lcd_send_string("INIT");
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
