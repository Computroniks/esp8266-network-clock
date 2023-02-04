// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "timekeeping/clock.hpp"

extern "C" void app_main() {
    ESP_LOGI("STARTUP", "%s %s", PROJECT_NAME, PROJECT_VERSION);
    ESP_LOGI("STARTUP", "Compiled %s %s", __DATE__, __TIME__);

    // Log chip info
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI("APP_MAIN", "This is an ESP8266 chip with %d CPU cores", chip_info.cores);
    ESP_LOGI("APP_MAIN", "Silicon revision %d", chip_info.revision);
    ESP_LOGI("APP_MAIN", "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    timekeeping::Clock clock(1675512811);
    for (int i = 10; i >= 0; i--) {
        ESP_LOGI("APP_MAIN", "Restarting in %d seconds...\n", i);
        clock.Now();
        ESP_LOGI("TIME", "Formatted time %d:%d:%d", clock.Hour(), clock.Minute(), clock.Second());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI("APP_MAIN", "Restarting now");
    esp_restart();
}
