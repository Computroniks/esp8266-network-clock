// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#include "segment.hpp"

#include "esp_log.h"

namespace display {
    Segment::Segment(int len) {
        max_chars_ = len;
    }
}
