/* Servo Motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/mcpwm.h"
#include "server_motor.h"
#include <sys/time.h>

static const char *TAG = "SERVO";

#define SERVO_MIN_PULSEWIDTH_US (600) // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US (2400) // Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE        (70)   // Maximum angle in degree upto which servo can rotate

#define SERVO_PULSE_GPIO        (18)   // GPIO connects to the PWM signal line

extern uint8_t scram_active;
extern QueueHandle_t FilaInterrupt;


static inline uint32_t example_convert_servo_angle_to_duty_us(int angle)
{
    return (angle + SERVO_MAX_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}


void servo_activate(void* pvParams)
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PULSE_GPIO); // To drive a RC servo, one MCPWM generator is enough
    mcpwm_config_t pwm_config = {
        .frequency = 50, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,     // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    int angle = 0;
    uint8_t interrupt;
    ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, example_convert_servo_angle_to_duty_us(angle)));
    
    while(1) {
        
        if(xQueueReceive(FilaInterrupt, (void *) &interrupt, (100/portTICK_PERIOD_MS))) {
            for (; angle > -SERVO_MAX_DEGREE; angle-=10) {
                ESP_LOGI(TAG, "Angle of rotation: %d", angle);
                ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, example_convert_servo_angle_to_duty_us(angle)));
                vTaskDelay(pdMS_TO_TICKS(50)); //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation under 5V power supply
            }
            ESP_LOGI("SCRAM", "SCRAM ATIVADO. REINICIAR SISTEMA");
            break;
        }
        
    }
      
    vTaskDelete(NULL);
    
}
