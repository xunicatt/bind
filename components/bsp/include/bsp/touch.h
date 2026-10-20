// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H

#include <bsp/i2c.h>
#include <esp_err.h>
#include <esp_lcd_touch.h>

namespace bsp {
    class TouchInput {
    public:
        TouchInput(const TouchInput&) = delete;
        TouchInput& operator=(const TouchInput&) = delete;

        TouchInput() = default;
        ~TouchInput();

        esp_err_t init(I2c& i2c);
        void deinit();
        esp_err_t sleep(bool sleep);
        esp_lcd_touch_handle_t handle();

    private:
        esp_err_t reset();

        SemaphoreHandle_t sem_ = nullptr;
        esp_lcd_touch_handle_t handle_ = nullptr;
        esp_lcd_panel_io_handle_t io_ = nullptr;

        static constexpr auto I2C_ADDR = 0x38;
        static constexpr auto RST = GPIO_NUM_9;
        static constexpr auto INTR = GPIO_NUM_38;
        
        static constexpr auto PMODE_REG = 0xA5;
        static constexpr auto PMODE_ACTIVE = 0x00;
        static constexpr auto PMODE_HIBERNATE = 0x03;
    };
}

#endif
