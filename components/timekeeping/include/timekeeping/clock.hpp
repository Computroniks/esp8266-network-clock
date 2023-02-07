// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#ifndef TIMEKEEPING_CLOCK_H_
#define TIMEKEEPING_CLOCK_H_

#include <time.h>

#include "lwip/inet.h"

class Clock
{
private:
    time_t time_;
    const char* ntp_server_name_;
    const char TAG_[6] = "CLOCK";

    // Initialise SNTP
    void InitSNTP();

public:
    // Start the clock and sync with the specified NTP server
    Clock(const char* server);

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

    // Get the current time now
    time_t Now();
};


#endif  // TIMEKEEPING_CLOCK_H_
