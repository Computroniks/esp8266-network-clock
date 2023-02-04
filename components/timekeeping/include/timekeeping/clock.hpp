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

        // Get the hour value of the clock.
        // To get the current hour, first make sure to update the
        // clock by calling Now()
        int Hour();

        // Get the minute value of the clock. 
        // To get the current minute, first make sure to update the
        // clock by calling Now()
        int Minute();

        // Get the second value of the clock
        // To get the current second, first make sure to update the
        // clock by calling Now()
        int Second();

        // Set the clock to a given time
        void Set(time_t time);

        // Get the current time now
        time_t Now();
    };
}

#endif  // TIMEKEEPING_CLOCK_H_
