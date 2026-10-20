// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_RTC_H
#define BSP_RTC_H

#include <bsp/i2c.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <pcf85063a.h>

namespace bsp {
    class Rtc {
    public:
        Rtc(const Rtc&) = delete;
        Rtc& operator=(const Rtc&) = delete;

        Rtc() = default;
        ~Rtc();

        esp_err_t init(I2c& i2c);
        void deinit();
        pcf85063a_dev_t *dev();

    private:
        pcf85063a_dev_t dev_ = {};

        static constexpr auto I2C_ADDR = 0x51;
        static constexpr auto INTR = GPIO_NUM_39;
    };
}

#endif

