// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_HAPTIC_H
#define BSP_HAPTIC_H

#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_timer.h>

namespace bsp {
    class Haptic {
    public:
        Haptic(const Haptic&) = delete;
        Haptic& operator=(const Haptic&) = delete;

        Haptic() = default;
        ~Haptic();

        esp_err_t init();
        void deinit();
        void on();
        void off();
        void vibrate(uint32_t ms);
        void set_strength(uint8_t strength);

    private:
        uint32_t ms_;
        esp_timer_handle_t timer_ = nullptr;

        static constexpr auto HAPTIC = GPIO_NUM_18;
    };
}

#endif
