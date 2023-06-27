#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DIGITAL_PIN 15
#define MAX_OUTPUT 1000
#define SAFETY_MAX 500
#define MAX_VOLTAGE 2.2

extern void IRAM_ATTR gpio_isr_handle_func(void* args);
extern QueueHandle_t FilaInterrupt;
extern SemaphoreHandle_t SemaforoLumi;
extern float power_value;
extern uint8_t scram_active;

static const char *TAG = "Light sensor";
static esp_adc_cal_characteristics_t adc1_chars;

uint32_t traduz_para_lumens(uint32_t tensao) {
    return tensao;
}

void light_sensor(void* pvParams)
{
    uint32_t voltage;
    ESP_LOGI(TAG, "----LIGHT SENSOR START ----");
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11)); // Usa pino 32

    gpio_pad_select_gpio(DIGITAL_PIN);
    gpio_set_direction(DIGITAL_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(DIGITAL_PIN);
    gpio_pullup_dis(DIGITAL_PIN);
    gpio_set_intr_type(DIGITAL_PIN, GPIO_INTR_POSEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(DIGITAL_PIN, gpio_isr_handle_func, (void *)DIGITAL_PIN);

    while (1) 
    {
        voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_4), &adc1_chars);
        power_value = traduz_para_lumens(voltage);
        if(power_value >= SAFETY_MAX) {
            scram_active = 1;
            xQueueSend(FilaInterrupt, &scram_active, (20/portTICK_PERIOD_MS));
        }
        //ESP_LOGI(TAG, "ADC1_CHANNEL_4: %d mV", voltage);
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}