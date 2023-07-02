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
#include <sys/time.h>

static const char *TAG = "DHT";
extern QueueHandle_t FilaInterrupt;
extern SemaphoreHandle_t SemaforoTemp;
extern SemaphoreHandle_t SemaforoUmid;
extern uint8_t scram_active;

extern float temp_value;
extern float umid_value;
struct timespec tv_now;

void DHT_task(void *pvParameter)
{
    gettimeofday(&tv_now, NULL);
    ESP_LOGI("lcdtemp","init1: %lld", ((int64_t)tv_now.tv_sec*1000000LL)+(int64_t)tv_now.tv_nsec);
    setDHTgpio(GPIO_NUM_4);
    ESP_LOGI(TAG, "Starting DHT Task");
    gettimeofday(&tv_now, NULL);
    ESP_LOGI("lcdtemp","end1: %lld", ((int64_t)tv_now.tv_sec*1000000LL)+(int64_t)tv_now.tv_nsec);
    while (1)
    {
        gettimeofday(&tv_now, NULL);
        ESP_LOGI("lcdtemp","init2: %lld", ((int64_t)tv_now.tv_sec*1000000LL)+(int64_t)tv_now.tv_nsec);
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
        if((temp_value>=50 || umid_value>=10) && (!scram_active)) {
            ESP_LOGW("SCRAM:", "SCRAM BY TEMP SENSOR");
            scram_active = 1;
            xQueueSend(FilaInterrupt, &scram_active, (20/portTICK_PERIOD_MS));
        }
        // -- wait at least 2 sec before reading again ------------
        // The interval of whole process must be beyond 2 seconds !!
        gettimeofday(&tv_now, NULL);
        ESP_LOGI("lcdtemp","end2: %lld", ((int64_t)tv_now.tv_sec*1000000LL)+(int64_t)tv_now.tv_nsec);
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}
