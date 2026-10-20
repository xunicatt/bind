// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_RAMFS_H
#define BSP_RAMFS_H

#include <esp_err.h>

namespace bsp {
    class RamFs {
    public:
        RamFs(const RamFs&) = delete;
        RamFs& operator=(const RamFs&) = delete;

        RamFs() = default;
        ~RamFs();

        esp_err_t mount();
        void unmount();
        bool mounted();

        static constexpr const char *MOUNT_POINT = "/tmp";

    private:
        bool mounted_ = false;

        static constexpr auto MAX_PARTITION_SIZE = (256 * 1024);
        static constexpr auto MAX_FILES = 1024;
        static constexpr auto PRINT_INFO_ON_MOUNT = 1;
    };
}

#endif
