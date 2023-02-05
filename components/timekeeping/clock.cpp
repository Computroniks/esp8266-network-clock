// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "clock.hpp"

#include <time.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "lwip/apps/sntp.h"
#include "lwip/inet.h"

namespace timekeeping {
    void Clock::InitSNTP() {
        ESP_LOGI(TAG_, "Initialising SNTP");
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, ntp_server_name_);
        sntp_init();
        ESP_LOGI(
            TAG_,
            "Started SNTP client. Polling with interval %d ms. Using server %s.",
            sntp_get_sync_interval(),
            ntp_server_name_
        );
    }

    Clock::Clock(const char* server) {
        ntp_server_name_ = server;
        InitSNTP();
    }

    int Clock::Hour() {
        return (time_ / 3600) % 24;
    }

    int Clock::Minute() {
        return (time_ / 60) % 60;
    }

    int Clock::Second() {
        return time_ % 60;
    }

    time_t Clock::Now() {
        time(&time_);
        return time_;
    }
}
