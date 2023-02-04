// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "clock.hpp"

#include <time.h>

#include "esp_timer.h"

namespace timekeeping {
    Clock::Clock(time_t time) {
        Set(time);
    }

    int Clock::Hour() {
        return (last_time_ / 3600) % 24;
    }

    int Clock::Minute() {
        return (last_time_ / 60) % 60;
    }

    int Clock::Second() {
        return last_time_ % 60;
    }

    void Clock::Set(time_t time) {
        last_time_ = time;
        last_millis_ = esp_timer_get_time() / 1000UL;
    }

    time_t Clock::Now() {
        // Calculate number of milliseconds passed since last call and
        // use this to calculate increase in time.

        int diff = (esp_timer_get_time() / 1000UL) - last_millis_;
        last_millis_ = esp_timer_get_time() / 1000UL;
        last_time_ += diff / 1000UL;

        return last_time_;
    }
}
