// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/ramfs.h>
#include <esp_check.h>
#include <esp_log.h>
#include <ramfs.h>

namespace bsp {
    static const char *TAG = "bsp::ramfs";

    esp_err_t RamFs::mount() {
        if (mounted()) {
            return ESP_OK;
        }

        esp_err_t ret = ESP_OK;

        const ramfs_config_t config = {
            .base_path = MOUNT_POINT,
            .max_files = MAX_FILES,
            .max_bytes = MAX_PARTITION_SIZE,
            .caps = MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM,
        };
        ret = ramfs_register(&config);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to mount ramfs");

        if (PRINT_INFO_ON_MOUNT == 1) {
            size_t total, used;
            ret = ramfs_info(MOUNT_POINT, &total, &used);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to get ramfs info");

            ESP_LOGI(TAG, "mounted ramfs at: %s", MOUNT_POINT);
            ESP_LOGI(TAG, "RAMFS:");
            ESP_LOGI(TAG, "\tCapacity: %zu KB", total >> 10);
            ESP_LOGI(TAG, "\tUsed : %zu KB", used >> 10);
        }

        mounted_ = true;

        return ESP_OK;

    err:
        ramfs_unregister(MOUNT_POINT);

        return ret;
    }

    void RamFs::unmount() {
        if (mounted()) {
            ramfs_unregister(MOUNT_POINT);
            mounted_ = false;
        }
    }

    RamFs::~RamFs() {
        unmount();
    }

    bool RamFs::mounted() {
        return mounted_;
    }
}
