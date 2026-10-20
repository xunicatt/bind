// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <esp_check.h>
#include <nvs_flash.h>
#include <state.h>

#define STATE_LOAD(nvs, type, key, var, val)    \
({                                              \
    esp_err_t ret = ESP_OK;                     \
    ret = nvs_get_##type((nvs), (key), &(var)); \
    if (ret == ESP_ERR_NVS_NOT_FOUND) {         \
        (var) = (val);                          \
        ret = ESP_OK;                           \
    };                                          \
    ret;                                        \
})

namespace os {
    static const char *TAG = "bindos::state";

    esp_err_t State::init() {
        esp_err_t ret = ESP_OK;

        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ret = nvs_flash_erase();
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to format: nvs");
        } else {
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize: nvs");
        }

        return ESP_OK;
    }

    void State::deinit() {
        nvs_flash_deinit();
    }

    esp_err_t State::load() {
        esp_err_t ret = ESP_OK;

        nvs_handle_t nvs = 0;
        ret = nvs_open("configs", NVS_READONLY, &nvs);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to open: configs");

        ret = STATE_LOAD(nvs, u8, BRIGHTNESS, brightness, BRIGHTNESS_DEFAULT);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to load key: %s", BRIGHTNESS);

        nvs_close(nvs);

        return ESP_OK;
    }

    esp_err_t State::save() {
        esp_err_t ret = ESP_OK;

        nvs_handle_t nvs = 0;
        ret = nvs_open("configs", NVS_READWRITE, &nvs);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to open: configs");

        ret = nvs_set_u8(nvs, BRIGHTNESS, brightness);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to save key: %s", BRIGHTNESS);

        nvs_close(nvs);

        return ESP_OK;
    }
}
