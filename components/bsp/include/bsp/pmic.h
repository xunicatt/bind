// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_PMIC_H
#define BSP_PMIC_H

#include <bsp/i2c.h>
#include <esp_err.h>

namespace bsp {
    class Pmic {
    public:
        Pmic(const Pmic&) = delete;
        Pmic& operator=(const Pmic&) = delete;

        Pmic() = default;
        ~Pmic();

        esp_err_t init(I2c& i2c);
        void deinit();
        esp_err_t poweroff();

    private:
        static constexpr auto I2C_ADDR = 0x34;
        static constexpr auto SHUTDOWN_ON_LONG_PRESS = 1;
    };
}

#endif
