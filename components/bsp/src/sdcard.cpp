// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/sdcard.h>
#include <esp_check.h>
#include <esp_vfs_fat.h>

namespace bsp {
    static const char *TAG = "bsp::sdcard";

    esp_err_t SDCard::mount() {
        if (sdmmc_ != nullptr) {
            return ESP_OK;
        }

        esp_err_t ret = ESP_OK;

        const sdmmc_host_t host = SDSPI_HOST_DEFAULT();

        {
            const spi_bus_config_t config = {
                .mosi_io_num = MOSI,
                .miso_io_num = MISO,
                .sclk_io_num = CLK,
                .quadwp_io_num = GPIO_NUM_NC,
                .quadhd_io_num = GPIO_NUM_NC,
                .data4_io_num = GPIO_NUM_NC,
                .data5_io_num = GPIO_NUM_NC,
                .data6_io_num = GPIO_NUM_NC,
                .data7_io_num = GPIO_NUM_NC,
                .data_io_default_level = false,
                .max_transfer_sz = MAX_TRANSFER_SIZE,
                .flags = 0,
                .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
                .intr_flags = 0,
            };
            ret = spi_bus_initialize(
                static_cast<spi_host_device_t>(host.slot),
                &config, SDSPI_DEFAULT_DMA
            );
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialze bus");
        }

        {
            const esp_vfs_fat_sdmmc_mount_config_t config = {
                .format_if_mount_failed = false,
                .max_files = MAX_FILES,
                .allocation_unit_size = ALLOCATION_SIZE,
                .disk_status_check_enable = false,
                .use_one_fat = false,
            };

            sdspi_device_config_t sdspi_config = SDSPI_DEVICE_CONFIG_DEFAULT();
            sdspi_config.gpio_cs = CS;
            sdspi_config.host_id = static_cast<spi_host_device_t>(host.slot);

            ret = esp_vfs_fat_sdspi_mount(
                MOUNT_POINT, &host, &sdspi_config,
                &config, &sdmmc_
            );
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to mount sd card");
        }

        if (PRINT_INFO_ON_MOUNT == 1) {
            const uint64_t capacity = (
                (uint64_t)sdmmc_->csd.capacity * sdmmc_->csd.sector_size
            ) >> 10;
            ESP_LOGI(TAG, "mounted sdcard at: %s", MOUNT_POINT);
            ESP_LOGI(TAG, "SDCARD:");
            ESP_LOGI(TAG, "\tName: %s", sdmmc_->cid.name);
            ESP_LOGI(TAG, "\tCapacity: %llu KB", capacity);
            ESP_LOGI(TAG, "\tSector Size: %u", sdmmc_->csd.sector_size);
            ESP_LOGI(TAG, "\tFrequency: %u Khz", sdmmc_->max_freq_khz);
        }

        mounted_ = true;

        return ESP_OK;

    err:
        spi_bus_free(static_cast<spi_host_device_t>(host.slot));

        return ret;
    }

    void SDCard::unmount() {
        if (!mounted()) {
            return;
        }

        esp_vfs_fat_sdcard_unmount(MOUNT_POINT, sdmmc_);
        sdmmc_ = nullptr;
        mounted_ = false;

        spi_bus_free(SDSPI_DEFAULT_HOST);
    }

    SDCard::~SDCard() {
        unmount();
    }

    bool SDCard::mounted() {
        return mounted_;
    }
}
