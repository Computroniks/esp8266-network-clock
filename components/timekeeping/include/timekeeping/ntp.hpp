// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#ifndef TIMEKEEPING_NTP_H_
#define TIMEKEEPING_NTP_H_

#include <time.h>

#include "lwip/inet.h"

namespace timekeeping {
    class NTP
    {
    private:
        in_addr_t server_;
    public:
        NTP(in_addr_t server);

        // Poll server to get current time
        time_t Sync();
    };
}

#endif  // TIMEKEEPING_NTP_H_
