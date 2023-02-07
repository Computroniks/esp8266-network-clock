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
#include "freertos/semphr.h"

// Delay used for hardware timer in us (microseconds)
// Must be > 50
#define CLK_DELAY 51

// Maximum time to wait before failing send
#define MAX_SEND_TIMEOUT 200

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
    const TickType_t max_block_time = pdMS_TO_TICKS(MAX_SEND_TIMEOUT);

    // Set of the timer
    hw_timer_init(SendByteISR, this);
    hw_timer_alarm_us(CLK_DELAY, true);

    // Wait until finished writing to display
    int result = xSemaphoreTake(write_semaphore_, max_block_time);

    if (result != pdTRUE) {
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
        // 3 * 8 bits + extra bit for ACK
        if (t->counter_ > 27) {
            // We have sent the last bit
            BaseType_t higher_priority_task_woken = pdFALSE;
            xSemaphoreGiveFromISR(
                t->write_semaphore_,
                &higher_priority_task_woken
            );
            portEND_SWITCHING_ISR(higher_priority_task_woken);
        }
        break;
    }
    t->counter_++;
}

void TM1637::Start() {
    counter_ = 0;

    // Task notification setup
    const TickType_t max_block_time = pdMS_TO_TICKS(MAX_SEND_TIMEOUT);

    // Set of the timer
    hw_timer_init(StartISR, this);
    hw_timer_alarm_us(CLK_DELAY, true);

    // Wait until finished writing to display
    int result = xSemaphoreTake(write_semaphore_, max_block_time);

    if (result != pdTRUE) {
        ESP_LOGE(
            TAG_,
            "Failed to send start. Function timed out after 200ms"
        );
    }

    hw_timer_deinit();
}

void TM1637::StartISR(void* arg) {
    TM1637* t = (TM1637*)arg;
    if (t->counter_ % 2 == 0) {
        gpio_set_level(t->dio_, 0);
    }
    else {
        // Note, there is no need to set clock to low here as this is
        // done first thing when sending a byte. All we do here is give
        // back the semaphore to continue execution.
        BaseType_t higher_priority_task_woken = pdFALSE;
        xSemaphoreGiveFromISR(
            t->write_semaphore_,
            &higher_priority_task_woken
        );
        portEND_SWITCHING_ISR(higher_priority_task_woken);
    }
    t->counter_++;
}

void TM1637::Stop() {
    counter_ = 0;

    // Task notification setup
    const TickType_t max_block_time = pdMS_TO_TICKS(MAX_SEND_TIMEOUT);

    // Set of the timer
    hw_timer_init(StopISR, this);
    hw_timer_alarm_us(CLK_DELAY, true);

    // Wait until finished writing to display
    int result = xSemaphoreTake(write_semaphore_, max_block_time);

    if (result != pdTRUE) {
        ESP_LOGE(
            TAG_,
            "Failed to send start. Function timed out after 200ms"
        );
    }

    hw_timer_deinit();
}

void TM1637::StopISR(void* arg) {
    TM1637* t = (TM1637*)arg;

    switch (t->counter_ % 4) {
    case 0: // Clock falling
        gpio_set_level(t->clk_, 0);
        break;
    case 1: // Clock low, data is still low from ack
        break;
    case 2: // Clock goes high
        gpio_set_level(t->clk_, 1);
        break;
    case 3: // Clock is now high, data can go high
        gpio_set_level(t->dio_, 1);
        BaseType_t higher_priority_task_woken = pdFALSE;
        xSemaphoreGiveFromISR(
            t->write_semaphore_,
            &higher_priority_task_woken
        );
        portEND_SWITCHING_ISR(higher_priority_task_woken);
        break;
    }
    t->counter_++;
}

TM1637::TM1637(int dio, int clk):Segment(6) {
    dio_ = (gpio_num_t)dio;
    clk_ = (gpio_num_t)clk;

    // Create our semaphore that will be used later
    write_semaphore_ = xSemaphoreCreateBinary();

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
