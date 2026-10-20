// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <esp_check.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <updater.h>
#include <sys/stat.h>

namespace rebind {
    static const char *TAG = "rebind::update";

    constexpr size_t BUFFER_SIZE = (4 * 1024);
    static uint8_t buffer[BUFFER_SIZE];

    Updater::Updater(Board &board): board_(board) {}

    esp_err_t Updater::init(GuiNotifyFn notify) {
        struct stat filestat = {};
        esp_err_t ret = ESP_OK;
        notify_ = notify;

        status_ = Status::Fetching;
        progress_ = 0;
        size_ = 0;

        if (stat(UPDATE_DONE_FILE, &filestat) == 0) {
            ESP_LOGI(TAG, "update is already applied");
            status_ = Status::UpdateApplied;
        } else if (stat(OS_UPDATE_FILE, &filestat) == 0) {
            ESP_LOGI(TAG, "found update file: %s", OS_UPDATE_FILE);
            size_ = filestat.st_size;
            status_ = Status::UpdateAvailable;
        } else {
            status_ = Status::UpdateUnavailable;
            notify_();
            return ESP_OK;
        }

        notify_();

        if (status_ == Status::UpdateAvailable) {
            status_ = Status::Updating;
            notify_();
            ret = apply();
            if (ret != ESP_OK) {
                status_ = Status::UpdateFailed;
                notify_();
                ESP_RETURN_ON_ERROR(
                    ret, TAG, "failed to apply update: %s", esp_err_to_name(ret)
                );
            }

            status_ = Status::UpdateCompleted;
            notify_();

            board_.reboot();
        }

        return ESP_OK;
    }

    esp_err_t Updater::begin() {
        assert(notify_ != nullptr);

        remove(UPDATE_DONE_FILE);
        return init(notify_);
    }

    Updater::Status Updater::stauts() {
        return status_;
    }

    uint8_t Updater::progress() {
        return progress_;
    }

    esp_err_t Updater::apply() {
        esp_err_t ret = ESP_OK;
        FILE *file = nullptr;
        esp_ota_handle_t update = 0;
        size_t read = 0, written = 0;

        ESP_RETURN_ON_FALSE(size_ != 0, ESP_FAIL, TAG, "update file is empty");

        const esp_partition_t *os_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP,
            ESP_PARTITION_SUBTYPE_APP_OTA_0,
            nullptr 
        );
        ESP_RETURN_ON_FALSE(
            os_partition != nullptr, ESP_ERR_NOT_FOUND,
            TAG, "failed to get app ota1 partition"
        );

        file = fopen(OS_UPDATE_FILE, "rb");
        ESP_RETURN_ON_FALSE(
            file != nullptr, ESP_ERR_NOT_FOUND,
            TAG, "failed to open update file"
        );

        ret = esp_ota_begin(os_partition, OTA_WITH_SEQUENTIAL_WRITES, &update);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to begin ota update");

        while ((read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            ret = esp_ota_write(update, (const void *)buffer, read);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to write update");

            written += read;
            progress_ = (written * 100) / size_;

            ESP_LOGI(TAG, "update percentage: %d", progress_);
        }

        esp_ota_end(update);
        fclose(file);

        file = fopen(UPDATE_DONE_FILE, "w");
        fclose(file);

        ret = esp_ota_set_boot_partition(os_partition);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set boot partition to os");

        return ESP_OK;

    err:
        if (update != 0) {
            esp_ota_abort(update);
            update = 0;
        }

        if (file != nullptr) {
            fclose(file);
            file = nullptr;
        }

        return ret;
    }
}
