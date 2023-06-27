/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
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
//#include "lcd_driver.h"
#include "temp_driver.h"

#define I2C_MASTER_SCL_IO           19          /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           18          /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0           /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          100000      /*!< I2C master clock frequency */
#define I2C_MASTER_TIMEOUT_MS       1000
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

void task_teste(void* pvParams) {
    while(!scram_active) {
        printf("Essa é uma task de teste\n");
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}


void IRAM_ATTR gpio_isr_handle_func(void* args) {
    if(!scram_active) {
        scram_active = 1;
        xQueueSend(FilaInterrupt, &scram_active, (200/portTICK_PERIOD_MS));
    }
}

void app_main(void)
{
    TaskHandle_t teste;
    TaskHandle_t servoTask;
    SemaforoTemp = xSemaphoreCreateBinary();
    xSemaphoreGive(SemaforoTemp);
    SemaforoUmid = xSemaphoreCreateBinary();
    xSemaphoreGive(SemaforoUmid);
    SemaforoLumi = xSemaphoreCreateBinary();
    xSemaphoreGive(SemaforoLumi);
    //xTaskCreate(taskLCD, "taskLCD", 1024*4, &lcdTask, 12, &lcdTask);
    xTaskCreate(DHT_task, "DHT_task", 1024*4, NULL, 12, NULL);
    FilaInterrupt = xQueueCreate(1, sizeof(uint8_t));

    gpio_pad_select_gpio(BUTTON_PIN);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON_PIN);
    gpio_pullup_dis(BUTTON_PIN);
    gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, gpio_isr_handle_func, (void *)BUTTON_PIN);
    
    xTaskCreate(servo_activate, "servo_activate", 1024*2, &servoTask, configMAX_PRIORITIES-3, &servoTask);
    xTaskCreate(task_teste, "task_teste", 1024, &teste, 10, &teste);
    flag_init = 1;
    //esp_restart();

}
