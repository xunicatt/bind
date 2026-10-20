// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef INIT_H
#define INIT_H

#include <esp_err.h>

namespace os {
    esp_err_t init();
    void logo();
    void poweroff();
    void reboot();
}

#endif
