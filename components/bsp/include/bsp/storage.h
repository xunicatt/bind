// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_STORAGE_H
#define BSP_STORAGE_H

#include <esp_err.h>

namespace bsp {
    class Storage {
    public:
        Storage(const Storage&) = delete;
        Storage& operator=(const Storage&) = delete;

        Storage() = default;
        ~Storage();

        esp_err_t mount();
        void unmount();
        bool mounted();

        static constexpr const char *MOUNT_POINT = "/storage";

    private:
        bool mounted_ = false;

        static constexpr auto MAX_FILES = 1024;
        static constexpr auto PRINT_INFO_ON_MOUNT = 1;
    };
}

#endif

