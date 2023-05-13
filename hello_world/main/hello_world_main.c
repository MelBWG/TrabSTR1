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
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"


void task_teste() {
    while(true) {
        printf("Essa é uma task de teste");
        ESP_LOGW("TASK_TESTE", "Mantenha a calma");
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}


void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    xTaskCreate(task_teste, "teste_task", 1024*3, NULL, 10, NULL);
    char buffer[300];
    vTaskList(buffer);
    ESP_LOGI("MAIN", "%s",  buffer);
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskList(buffer);
         ESP_LOGI("MAIN", "%s",  buffer);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskList(buffer);
     ESP_LOGI("MAIN", "%s",  buffer);
    

    fflush(stdout);
    printf("Restarting now.\n");
    esp_restart();

}
