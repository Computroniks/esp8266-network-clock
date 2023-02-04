// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "clock.hpp"

#include <time.h>

namespace timekeeping {
    Clock::Clock(time_t time) {
        Set(time);
    }

    int Clock::Hour() {
        Now();
        return 0;
    }

    int Clock::Minute() {
        Now();
        return 0;
    }

    int Clock::Second() {
        Now();
        return 0;
    }

    int Clock::MilliSecond() {
        Now();
        return 0;
    }

    void Clock::Set(time_t time) {
        last_time_ = time;
    }

    time_t Clock::Now() {
        /* Calculate number of milliseconds passed since last call and
        use this to calculate increase in time. */
        return last_time_;
    }
}
