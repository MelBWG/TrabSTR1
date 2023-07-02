#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "temp_driver.h"
#include "DHT.h"

static const char *TAG = "DHT";
extern QueueHandle_t FilaInterrupt;
extern SemaphoreHandle_t SemaforoTemp;
extern SemaphoreHandle_t SemaforoUmid;
extern uint8_t scram_active;

extern float temp_value;
extern float umid_value;


void DHT_task(void *pvParameter)
{
    setDHTgpio(GPIO_NUM_4);
    ESP_LOGI(TAG, "Starting DHT Task\n\n");
    while (1)
    {
        //ESP_LOGI(TAG, "=== Reading DHT ===\n");
        int ret = readDHT();
        //ESP_LOGW("RESP TEMP:","Valor ret: %d",ret);
        if(xSemaphoreTake(SemaforoTemp, (TickType_t) 100) == pdTRUE) {
            temp_value = getTemperature();
            xSemaphoreGive(SemaforoTemp);
        }
        if(xSemaphoreTake(SemaforoUmid, (TickType_t) 100) == pdTRUE) {
            umid_value = getHumidity();
            xSemaphoreGive(SemaforoUmid);
        }
        //ESP_LOGI(TAG, "Hum: %.1f Tmp: %.1f\n", umid_value, temp_value);
        if((temp_value>=50 || umid_value>=90) && (!scram_active)) {
            ESP_LOGW("SCRAM:", "SCRAM BY TEMP SENSOR");
            scram_active = 1;
            xQueueSend(FilaInterrupt, &scram_active, (20/portTICK_PERIOD_MS));
        }
        // -- wait at least 2 sec before reading again ------------
        // The interval of whole process must be beyond 2 seconds !!
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}
