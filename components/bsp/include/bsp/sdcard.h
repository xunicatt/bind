// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_SDCARD_H
#define BSP_SDCARD_H

#include <driver/gpio.h>
#include <driver/sdspi_host.h>
#include <driver/spi_master.h>
#include <esp_err.h>

namespace bsp {
    class SDCard {
    public:
        SDCard(const SDCard&) = delete;
        SDCard& operator=(const SDCard&) = delete;

        SDCard() = default;
        ~SDCard();

        esp_err_t mount();
        void unmount();
        bool mounted();

        static constexpr const char *MOUNT_POINT = "/sdcard";

    private:
        bool mounted_ = false;
        sdmmc_card_t *sdmmc_ = NULL;

        static constexpr auto MAX_FILES = 1024;
        static constexpr auto ALLOCATION_SIZE = (32 * 1024);
        static constexpr auto MAX_TRANSFER_SIZE = (16 * 1024);

        static constexpr auto SPI = SPI2_HOST;
        static constexpr auto MISO = GPIO_NUM_3;
        static constexpr auto MOSI = GPIO_NUM_1;
        static constexpr auto CLK = GPIO_NUM_2;
        static constexpr auto CS = GPIO_NUM_17;

        static constexpr auto PRINT_INFO_ON_MOUNT = 1;
    };
}

#endif

