#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "freertos/queue.h"
#include "esp_log.h"

//H Bridge driver declarations
#define IN1 GPIO_NUM_3
#define IN2 GPIO_NUM_5
#define EN1 GPIO_NUM_4
#define EN2 GPIO_NUM_6
#define nRESET GPIO_NUM_34
#define nSLEEP GPIO_NUM_33

//PWM Controller declarations
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (21) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (0) // 8192 for 100%. To be decided.
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

int strength_current = 0;
int STIMStrength[] = {0, 819, 1638, 2457, 3276, 4095, 4914, 5733, 6552, 7371, 8190};

static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = LEDC_DUTY, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void STIMTask(void *params)
{
    example_ledc_init();
    gpio_set_direction(IN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(IN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(EN1, GPIO_MODE_OUTPUT);
    gpio_set_direction(EN2, GPIO_MODE_OUTPUT);
    gpio_set_direction(nRESET, GPIO_MODE_OUTPUT);
    gpio_set_direction(nSLEEP, GPIO_MODE_OUTPUT);
    gpio_set_level(nRESET, 1);
    gpio_set_level(nSLEEP, 1);
    gpio_set_level(EN1, 1);
    gpio_set_level(EN2, 1);
    strength_current = 819;


    while(1)
    {

            // ets_printf("Driver on, strength set : %d \n", strength_current);
            for(int i= 0;i<20;i++)
            {
    
                gpio_set_level(IN1, 1);
                gpio_set_level(IN2, 0);
                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, strength_current));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
                vTaskDelay(10 / portTICK_PERIOD_MS);
                gpio_set_level(IN1, 0);
                gpio_set_level(IN2, 1);
                vTaskDelay(10 / portTICK_PERIOD_MS);

                ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
                ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));

                gpio_set_level(IN1, 0);
                gpio_set_level(IN2, 0);
                vTaskDelay(12 / portTICK_PERIOD_MS);
                

            }
            // ets_printf("Stimulation fired : driver off for 2 sec\n");
            // ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
            // ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
            vTaskDelay(2000 / portTICK_PERIOD_MS);
     }
    
}

void app_main(void)
{   
    example_ledc_init();
    xTaskCreate(STIMTask, "STIMTask", 2048, NULL, 1, NULL);
}
