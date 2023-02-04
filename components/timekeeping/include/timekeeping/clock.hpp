// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#ifndef TIMEKEEPING_CLOCK_H_
#define TIMEKEEPING_CLOCK_H_

#include <time.h>

namespace timekeeping {
    class Clock
    {
    private:
        int last_millis_;
        time_t last_time_;
    public:
        // Start the clock at the given time
        Clock(time_t time);

        // Get the current hour value of the clock
        int Hour();

        // Get the current minute value of the clock
        int Minute();

        // Get the current second value of the clock
        int Second();

        // Get the current millisecond value of the clock
        int MilliSecond();

        // Set the clock to a given time
        void Set(time_t time);

        // Get the current time now
        time_t Now();
    };
}

#endif  // TIMEKEEPING_CLOCK_H_
