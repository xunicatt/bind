// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <esp_check.h>
#include <esp_err.h>
#include <init.h>

static const char *TAG = "bindos::main";

extern "C" int app_main(void) {
    esp_err_t ret = ESP_OK;

    ret = os::init();
    ESP_RETURN_ON_ERROR(ret, TAG, "os::init failed!");

    return ESP_OK;
}

