#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "NRF24L01.h"

typedef union
{
    uint8_t value[32];
    char now_time[32];
} MYDATA_t;
MYDATA_t mydata;
void primary(void *pvParameters)
{
    ESP_LOGI(pcTaskGetName(0), "Start");
    NRF24_t dev;
    Nrf24_init(&dev);
    uint8_t payload  = sizeof(mydata.value);
    uint8_t channel = 90;
    Nrf24_config(&dev, channel, payload );
    esp_err_t ret = Nrf24_setRADDR(&dev, (uint8_t *)"ABCDE");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
        while (1)
        {
            vTaskDelay(1);
        }
    }

    // Set the receiver address using 5 characters
    ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK)
    {
        ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
        while (1)
        {
            vTaskDelay(1);
        }
    }

    // Print settings
    Nrf24_printDetails(&dev);

    while (1)
    {
        sprintf(mydata.now_time, "now is %d", xTaskGetTickCount());
        TickType_t startTick = xTaskGetTickCount();
        Nrf24_send(&dev, mydata.value);
        vTaskDelay(1);
        ESP_LOGI(pcTaskGetName(0), "Wait for sending.....");
        if (Nrf24_isSend(&dev, 1000))
        {
            ESP_LOGI(pcTaskGetName(0), "Send success:%s", mydata.now_time);

            // Wait for response
            // ESP_LOGI(pcTaskGetName(0), "Wait for response.....");
            memset(mydata.now_time, 0, sizeof(mydata.now_time));
            while (1)
            {
                if (Nrf24_dataReady(&dev))
                    break;
                vTaskDelay(1);
            }
            Nrf24_getData(&dev, mydata.value);
            TickType_t diffTick = xTaskGetTickCount() - startTick;
            ESP_LOGI(pcTaskGetName(0), "Got response:%s Elapsed:%d ticks", mydata.now_time, diffTick);
        }
        else
        {
            ESP_LOGW(pcTaskGetName(0), "Send fail:");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void app_main(void)
{
    xTaskCreate(primary, "PRIMARY", 1024*3, NULL, 2, NULL);
}
