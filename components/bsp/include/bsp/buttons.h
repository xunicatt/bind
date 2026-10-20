// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_BUTTONS_H
#define BSP_BUTTONS_H

#include <driver/gpio.h>
#include <esp_err.h>
#include <iot_button.h>

namespace bsp {
    class Buttons {
    public:
        Buttons(const Buttons&) = delete;
        Buttons& operator=(const Buttons&) = delete;

        Buttons() = default;
        ~Buttons();

        esp_err_t init();
        void deinit();
        button_handle_t pwr();
        button_handle_t boot();

    private:
        button_handle_t pwr_ = nullptr;
        button_handle_t boot_ = nullptr;

        static constexpr auto PWR_BUTTON = GPIO_NUM_10;
        static constexpr auto BOOT_BUTTON = GPIO_NUM_0;
    };
}

#endif
