/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "server_motor.h"
#include "lcd_driver.h"
#include "temp_driver.h"
#include "luz_driver.h"


#define BUTTON_PIN 5

uint8_t scram_active = 0; 

QueueHandle_t FilaInterrupt;

SemaphoreHandle_t SemaforoTemp = NULL;
SemaphoreHandle_t SemaforoUmid = NULL;
SemaphoreHandle_t SemaforoLumi = NULL;

uint8_t flag_init = 0;

float temp_value = 0.0;
float umid_value = 0.0;
uint32_t power_value = 0;


void IRAM_ATTR gpio_isr_handle_func(void* args) {
    if(!scram_active) {
        scram_active = 1;
        //ESP_LOGW("SCRAM:", "SCRAM BY GPIO");
        xQueueSend(FilaInterrupt, &scram_active, (200/portTICK_PERIOD_MS));
    }
}

void app_main(void)
{
    
    TaskHandle_t teste, lcd;
    TaskHandle_t servoTask;
    SemaforoTemp = xSemaphoreCreateBinary();
    xSemaphoreGive(SemaforoTemp);
    SemaforoUmid = xSemaphoreCreateBinary();
    xSemaphoreGive(SemaforoUmid);
    SemaforoLumi = xSemaphoreCreateBinary();
    xSemaphoreGive(SemaforoLumi);
    gpio_install_isr_service(0);
    FilaInterrupt = xQueueCreate(3, sizeof(uint8_t));
    xTaskCreate(taskLCD, "taskLCD", 1024*2, &lcd, 10, &lcd);

    xTaskCreate(light_sensor, "light_sensor", 1024*2, &teste, 14, &teste);

    xTaskCreate(DHT_task, "DHT_task", 1024*4, NULL, 12, NULL);

    gpio_pad_select_gpio(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON_PIN);
    gpio_pullup_dis(BUTTON_PIN);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_POSEDGE);
    
    gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handle_func, (void *)BUTTON_PIN);
    xTaskCreate(servo_activate, "servo_activate", 1024*2, &servoTask, configMAX_PRIORITIES-3, &servoTask);
    flag_init = 1;
}
