// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_err.h>
#include <esp_lcd_panel_dev.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace bsp {
    class Display {
    public:
        Display(const Display&) = delete;
        Display& operator=(const Display&) = delete;

        Display() = default;
        ~Display();

        esp_err_t init();
        void deinit();
        esp_lcd_panel_handle_t handle();
        void set_brightness(uint8_t percentage);

        static constexpr auto WIDTH = 410;
        static constexpr auto HEIGHT = 502;
        static constexpr auto BPP = 16;

    private:
        esp_lcd_panel_handle_t panel_ = nullptr;
        esp_lcd_panel_io_handle_t io_ = nullptr;
        SemaphoreHandle_t sem_ = nullptr;
        uint8_t brightness_ = DEFAULT_BRIGHTNESS;

        static constexpr auto SPI = SPI3_HOST;
        static constexpr auto TE = GPIO_NUM_13;
        static constexpr auto CS = GPIO_NUM_12;
        static constexpr auto RST = GPIO_NUM_8;
        static constexpr auto SCLK = GPIO_NUM_11;
        static constexpr auto SIO0 = GPIO_NUM_4;
        static constexpr auto SIO1 = GPIO_NUM_5;
        static constexpr auto SIO2 = GPIO_NUM_6;
        static constexpr auto SIO3 = GPIO_NUM_7;
        
        static constexpr auto X_OFFSET = 22;
        static constexpr auto Y_OFFSET = 0;

        static constexpr auto SPI_MASTER_FREQ = (60 * 1000 * 1000);
        static constexpr auto DEFAULT_BRIGHTNESS = 0;
    };
}

#endif
