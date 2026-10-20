// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/storage.h>
#include <esp_check.h>
#include <esp_littlefs.h>

namespace bsp {
    static const char *TAG = "bsp::storage";

    esp_err_t Storage::mount() {
        esp_err_t ret = ESP_OK;

        const esp_vfs_littlefs_conf_t config = {
            .base_path = MOUNT_POINT,
            .partition_label = "storage",
            .partition = nullptr,
            .format_if_mount_failed = false,
            .read_only = false,
            .dont_mount = false,
            .grow_on_mount = false,
        };

        if (mounted()) {
            return ESP_OK;
        }

        ret = esp_vfs_littlefs_register(&config);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to mount internal storage");

        if (PRINT_INFO_ON_MOUNT == 1) {
            size_t total, used;
            ret = esp_littlefs_info("storage", &total, &used);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to internal storage info");

            ESP_LOGI(TAG, "mounted internal storage at: %s", MOUNT_POINT);
            ESP_LOGI(TAG, "SPIFLASH:");
            ESP_LOGI(TAG, "\tCapacity: %zu KB", total >> 10);
            ESP_LOGI(TAG, "\tUsed : %zu KB", used >> 10);
        }

        mounted_ = true;

        return ESP_OK;

    err:
        esp_vfs_littlefs_unregister("storage");

        return ret;
    }

    void Storage::unmount() {
        if (!mounted_) {
            return;
        }

        esp_vfs_littlefs_unregister("storage");
        mounted_ = false;
    }

    Storage::~Storage() {
        unmount();
    }

    bool Storage::mounted() {
        return mounted_;
    }
}
