#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "HD44780.h"

#define LCD_ADDR 0x27
#define SDA_PIN  21
#define SCL_PIN  19
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

void taskLCD(void* param)
{
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    LCD_home();
    LCD_clearScreen();
    LCD_writeStr("System Init...");
    vTaskDelay(1500/portTICK_PERIOD_MS);
    LCD_home();
    LCD_clearScreen();
    char temp_buffer[3];
    char umid_buffer[3];
    char luz_buffer[4];
    LCD_home();
    LCD_writeStr("Temp:    Um:    ");
    LCD_setCursor(1, 1);
    LCD_writeStr("Pot:     S:     ");
    while (true) {
        
        if(flag_init) {
            if(xSemaphoreTake(SemaforoTemp, (TickType_t) 100)) {
                sprintf(temp_buffer, "%2d", (int) temp_value);
                LCD_setCursor(6, 0);
                LCD_writeStr(temp_buffer);
                xSemaphoreGive(SemaforoTemp);
            }
            if(xSemaphoreTake(SemaforoUmid, (TickType_t) 100)) {
                sprintf(umid_buffer, "%2d", (int) umid_value);
                LCD_setCursor(13, 0);
                LCD_writeStr(umid_buffer);
                xSemaphoreGive(SemaforoUmid);
            }
            if(xSemaphoreTake(SemaforoLumi, (TickType_t) 100)) {
                sprintf(luz_buffer, "%3d", (int) power_value);
                LCD_setCursor(5, 1);
                LCD_writeStr(luz_buffer);
                xSemaphoreGive(SemaforoLumi);
            }
            if(!scram_active) {
                LCD_setCursor(13, 1);
                LCD_writeStr("OK");
            }
            else {
                LCD_setCursor(12, 1);
                LCD_writeStr("SCRAM");
            }
        }
        else if(!scram_active) {
            LCD_setCursor(12, 1);
            LCD_writeStr("INIT");
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}
