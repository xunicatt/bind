// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef STATE_H
#define STATE_H

#include <esp_err.h>

namespace os {
    class State {
    public:
        esp_err_t init();
        void deinit();
        esp_err_t load();
        esp_err_t save();

        uint8_t brightness;

    private:
        static constexpr size_t LENGTH = 16;
        static constexpr const char BRIGHTNESS[LENGTH] = "disp.brightness";
        static constexpr uint8_t BRIGHTNESS_DEFAULT = 75;
    };
}

#endif
