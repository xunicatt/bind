// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_I2C_H
#define BSP_I2C_H

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <esp_err.h>

namespace bsp {
    class I2c {
    public:
        I2c(const I2c&) = delete;
        I2c& operator=(const I2c&) = delete;

        I2c() = default;
        ~I2c();

        esp_err_t init();
        void deinit();
        i2c_master_bus_handle_t handle();

        static constexpr auto PORT = I2C_NUM_0;

    private:
        i2c_master_bus_handle_t handle_ = nullptr;

        static constexpr auto SCL = GPIO_NUM_14;
        static constexpr auto SDA = GPIO_NUM_15;
    };
}

#endif
