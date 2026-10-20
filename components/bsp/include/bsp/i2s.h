// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_I2S_H
#define BSP_I2S_H

#include <driver/gpio.h>
#include <driver/i2s_std.h>
#include <esp_err.h>

namespace bsp {
    class I2s {
    public:
        I2s(const I2s&) = delete;
        I2s& operator=(const I2s&) = delete;

        I2s() = default;
        ~I2s();

        esp_err_t init();
        void deinit();
        i2s_chan_handle_t rx();
        i2s_chan_handle_t tx();

        static constexpr auto AUDIO_SAMPLE_RATE = (48 * 100);
        static constexpr auto PORT = I2S_NUM_0;

    private:
        i2s_chan_handle_t rx_ = nullptr;
        i2s_chan_handle_t tx_ = nullptr;

        static constexpr auto MCLK = GPIO_NUM_16;
        static constexpr auto BCLK = GPIO_NUM_41;
        static constexpr auto WS = GPIO_NUM_45;
        static constexpr auto DOUT = GPIO_NUM_40;
        static constexpr auto DIN = GPIO_NUM_42;
    };
}

#endif
