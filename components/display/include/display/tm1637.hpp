// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#ifndef DISPLAY_TM1367_H_
#define DISPLAY_TM1367_H_

#include "segment.hpp"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

namespace display {
    class TM1637: Segment
    {
    private:
        // Data in out pin
        gpio_num_t dio_;

        // Clock pin
        gpio_num_t clk_;

        // Tag to use for logging
        const char TAG_[16] = "DISPLAY::TM1637";

        // Used to work out where we are in the bit sending sequence
        int counter_ = 0;

        // Current data that is being sent to the IC
        int data_;

        // Task to notify on transmit complete
        TaskHandle_t task_notify_ = NULL;

        // Supported values for display
        //
        //      A
        //     ---
        //  F |   | B
        //     -G-
        //  E |   | C
        //     ---
        //      D   * H
        const int digits_[17] = {
            //HGFEDCBA
            0b00111111, // 0
            0b00000110, // 1
            0b01011011, // 2
            0b01001111, // 3
            0b01100110, // 4
            0b01101101, // 5
            0b01111101, // 6
            0b00000111, // 7
            0b01111111, // 8
            0b01101111, // 9
            0b01110111, // A
            0b01111100, // B
            0b00111001, // C
            0b01011110, // D
            0b01111001, // E
            0b01110001, // F
            0b10000000, // Decimal point
        };

        // Initialize the display
        void Init();

        // Send a single byte to the IC
        void SendByte(int b);

        // Send a single byte to the IC
        static void SendByteISR(void* arg);

        // Send start signal to TM1637
        void Start();

        // Callback for timer.
        // This is the method that actually writes the start bits to the device
        void StartISR(void* arg);

        // Send stop signal to TM1637
        void Stop();

        // Callback for stop timer
        void StopISR(void* arg);

    public:
        // Constructor. Set pins for data I/O and clock
        TM1637(int dio, int clk);

        void Write(char* msg);

        // For use in FreeRTOS tasks. Wait for a message to be sent via
        // the queue.
        void WaitForMsg(QueueHandle_t* queue);
    };
}

#endif  // DISPLAY_TM1367_H_
