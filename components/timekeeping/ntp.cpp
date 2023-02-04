// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "ntp.hpp"

#include <time.h>

#include "lwip/inet.h"

namespace timekeeping {
    NTP::NTP(in_addr_t server) {
        server_ = server;
        /* Do we need to do something here? */
    }

    time_t NTP::Sync() {
        /* Poll NTP server + parse response */
        return time_t();
    }
}
