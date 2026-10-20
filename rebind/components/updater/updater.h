// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef UPDATER_H
#define UPDATER_H

#include <board.h>
#include <esp_err.h>
#include <functional>

namespace rebind {
    class Updater {
    public:
        enum class Status {
            Fetching,
            Updating,
            UpdateAvailable,
            UpdateUnavailable,
            UpdateCompleted,
            UpdateApplied,
            UpdateFailed,
        };

        using GuiNotifyFn = std::function<void(void)>;

        Updater(Board &board);

        esp_err_t init(GuiNotifyFn notify);
        esp_err_t begin();
        Status stauts();
        uint8_t progress();

    private:
        esp_err_t apply();

        Board &board_;
        std::function<void(void)> notify_ = nullptr;
        uint8_t progress_ = 0;
        Status status_ = Status::Fetching;
        size_t size_ = 0;

        static constexpr const char *OS_UPDATE_FILE = "/sdcard/update/bindos.bin";
        static constexpr const char *FS_UPDATE_FILE = "/sdcard/update/storage.bin";
        static constexpr const char *UPDATE_DONE_FILE = "/sdcard/update/.done";
    };
}

#endif
