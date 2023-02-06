// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

#include "display/tm1637.hpp"
#include "timekeeping/clock.hpp"
#include "wifi_init.hpp"

QueueHandle_t display_queue;

void task_clock(void* arg) {
    timekeeping::Clock clock(get_ntp_server());

    // Some buffers for sending message to display
    char msg[4];

    for (;;) {
        clock.Now();
        msg[0] = (clock.Hour() / 10) % 10;
        msg[1] = clock.Hour() % 10;
        msg[2] = (clock.Minute() / 10) % 10;
        msg[3] = clock.Minute() % 10;
        xQueueSend(display_queue, msg, 0);
        ESP_LOGI(
            "TIME", "%d:%d:%d",
            clock.Hour(), clock.Minute(), clock.Second()
        );
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void task_display(void* arg) {
    display::TM1637 disp(0, 2);
    disp.WaitForMsg(&display_queue);
}

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
    vTaskDelay(CONFIG_STARTUP_DELAY / portTICK_PERIOD_MS);
    show_startup_info();
    network_init();

    display_queue = xQueueCreate(10, sizeof(char[5]));

    xTaskCreate(task_clock, "clock", 2048, NULL, 10, NULL);
    xTaskCreate(task_display, "display", 2048, NULL, 10, NULL);
}
