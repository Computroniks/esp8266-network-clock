// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "tm1637.hpp"

#include "driver/gpio.h"
#include "driver/hw_timer.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Delay used for hardware timer in us (microseconds)
#define CLK_DELAY 50

void TM1637::Init() {
    ESP_LOGI(TAG_, "Initialising TM1637");
    ESP_LOGI(
        TAG_,
        "Using following pins:\n\tDIO: %d\n\tCLK: %d",
        dio_,
        clk_
    );

    gpio_config_t config;
    config.intr_type = GPIO_INTR_DISABLE; // Disable interupts
    config.mode = GPIO_MODE_OUTPUT;
    config.pin_bit_mask = ((1ULL << dio_) | (1ULL << clk_));
    config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    config.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&config);

    // Both pins are expected high
    gpio_set_level(dio_, 1);
    gpio_set_level(clk_, 1);
}

void TM1637::SendByte(int b) {
    // Init our variables
    data_ = b;
    counter_ = 0;

    // Task notification setup
    const TickType_t max_block_time = pdMS_TO_TICKS(200); // Timeout 200ms
    task_notify_ = xTaskGetCurrentTaskHandle();


    // Set of the timer
    hw_timer_init(SendByteISR, this);
    hw_timer_alarm_us(CLK_DELAY, true);

    // Wait until finished writing to display
    int result = ulTaskNotifyTake(pdTRUE, max_block_time);

    if (result != 1) {
        ESP_LOGE(
            TAG_,
            "Failed to write byte to display. Function timed out after 200ms"
        );
    }

    hw_timer_deinit();
}

void TM1637::SendByteISR(void* arg) {
    TM1637* t = (TM1637*)arg;
    switch (t->counter_ % 3) {
    case 0: // Clock falling
        gpio_set_level(t->clk_, 0);
        break;
    case 1: // Clock low, set data bit
        gpio_set_level(t->dio_, t->data_ & 0x01);
        t->data_ = t->data_ >> 1;

        break;
    case 2: // Clock rising
        gpio_set_level(t->clk_, 1);
        if (t->counter_ > 24) {

            BaseType_t higher_priority_task_woken = pdFALSE;
            // We have set last bit
            vTaskNotifyGiveFromISR(
                t->task_notify_,
                &higher_priority_task_woken
            );

            t->task_notify_ = NULL;
            portEND_SWITCHING_ISR(higher_priority_task_woken);
        }
        break;
    }
    t->counter_++;
}

void TM1637::Start() {

}

void TM1637::StartISR(void* arg) {

}

void TM1637::Stop() {
    counter_ = 0;


}

void TM1637::StopISR(void* arg) {
    switch (counter_) {
    case 0:
        /* code */
        break;

    default:
        break;
    }

}

TM1637::TM1637(int dio, int clk):Segment(6) {
    dio_ = (gpio_num_t)dio;
    clk_ = (gpio_num_t)clk;

    Init();
}

void TM1637::Write(char* msg) {
    Start();
    SendByte(0b01000000); // Write to display with automatic addressing
    Stop();

    Start();
    SendByte(0xC0); // Address of first digit

    // Now send characters
    for (int i = 0; i < sizeof(&msg) / sizeof(msg[0]); i++) {
        ESP_LOGI(TAG_, "i = %d", i);
        ESP_LOGI(TAG_, "msg[%d] = %d", i, (int)msg[i]);
        ESP_LOGI(TAG_, "data = %d", digits_[(int)msg[i]]);
        /* TODO: Should probably check it is a valid char */
        if (i == 1) {
            // Include colon between hours and minutes
            SendByte(digits_[(int)msg[i]] & digits_[17]);
        }
        else {
            SendByte(digits_[(int)msg[i]]);
        }
    }
    Stop();

    Start();
    SendByte(0b10001111); // Display on, pulse width 14/16
    Stop();
}

void TM1637::WaitForMsg(QueueHandle_t* queue) {
    char msg[4];

    TickType_t last_wake;
    last_wake = xTaskGetTickCount();

    while (1) {
        if (xQueueReceive(*queue, msg, 0) == pdTRUE) {
            ESP_LOGI(TAG_, "Writing %s to display", msg);
            Write(msg);
        }
        vTaskDelayUntil(&last_wake, 100);
    }
}
