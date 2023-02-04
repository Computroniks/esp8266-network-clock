// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "timekeeping/clock.hpp"

// Output system information to the logging interface
void show_startup_info() {
    const char tag[] = "STARTUP";
    // Firmware info
    ESP_LOGI(tag, "%s %s", PROJECT_NAME, PROJECT_VERSION);
    ESP_LOGI(tag, "Compiled %s %s", __DATE__, __TIME__);
    ESP_LOGI(tag, "Repository %s\n", PROJECT_REPO);

    // IC Info
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    ESP_LOGI(
        tag, "Model: %s",
        (chip.model == CHIP_ESP8266) ? "ESP8266" : "ESP32"
    );
    ESP_LOGI(tag, "Silicon revision: %d", chip.revision);
    ESP_LOGI(tag, "Cores: %d", chip.cores);
    ESP_LOGI(tag, "Crystal: %dMHz\n", CRYSTAL_USED);

    // Chip features
    ESP_LOGI(tag, "Features:");
    ESP_LOGI(
        tag, "Embedded flash: %s",
        (chip.features & CHIP_FEATURE_EMB_FLASH) ? "Yes" : "No"
    );
    ESP_LOGI(
        tag, "2.4GHz WiFi: %s",
        (chip.features & CHIP_FEATURE_WIFI_BGN) ? "Yes" : "No"
    );
    ESP_LOGI(
        tag, "Bluetooth LE: %s",
        (chip.features & CHIP_FEATURE_BLE) ? "Yes" : "No"
    );
    ESP_LOGI(
        tag, "Bluetooth Classic: %s\n",
        (chip.features & CHIP_FEATURE_BT) ? "Yes" : "No"
    );

    // MAC addresses
    unsigned char mac[8];
    esp_efuse_mac_get_default(mac);
    ESP_LOGI(
        tag, "MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
    );

    // Flash Info
    ESP_LOGI(
        tag, "%dMB %s flash\n",
        spi_flash_get_chip_size() / (1024 * 1024),
        (chip.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external"
    );
}

extern "C" void app_main() {
    show_startup_info();
    timekeeping::Clock clock(1675512811);
    for (int i = 10; i >= 0; i--) {
        ESP_LOGI("APP_MAIN", "Restarting in %d seconds...\n", i);
        clock.Now();
        ESP_LOGI(
            "TIME", "Formatted time %d:%d:%d",
            clock.Hour(), clock.Minute(), clock.Second()
        );
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    ESP_LOGI("APP_MAIN", "Restarting now");
    esp_restart();
}
