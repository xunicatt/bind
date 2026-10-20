// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_IMU_H
#define BSP_IMU_H

#include <bsp/i2c.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <qmi8658.h>

namespace bsp {
    class Imu {
    public:
        Imu(const Imu&) = delete;
        Imu& operator=(const Imu&) = delete;

        Imu() = default;
        ~Imu();

        esp_err_t init(I2c& i2c);
        void deinit();
        qmi8658_dev_t *dev();

    private:
        qmi8658_dev_t dev_ = {};

        static constexpr auto I2C_ADDR = 0x6B;
        static constexpr auto INTR = GPIO_NUM_21;
    };
}

#endif
