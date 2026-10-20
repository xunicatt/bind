// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <board.h>
#include <esp_check.h>
#include <esp_system.h>

namespace rebind {
    static const char *TAG = "rebind::board";

    esp_err_t Board::init() {
        esp_err_t ret = ESP_OK;

        ret = buttons.init();
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize: buttons");
        ESP_LOGI(TAG, "initialized: buttons");

        ret = i2c.init();
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: i2c");
        ESP_LOGI(TAG, "initialized: i2c");

        ret = pmic.init(i2c);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: pmic");
        ESP_LOGI(TAG, "initialized: pmic");

        ret = haptic.init();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "failed to initialize: haptic");
            ret = ESP_OK;
        } else {
            ESP_LOGI(TAG, "initialized: haptic");
            haptic.vibrate(80);
        }

        ret = sdcard.mount();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to mount sdcard: %s", esp_err_to_name(ret));
            ret = ESP_OK;
        } else {
            ESP_LOGI(TAG, "mounted: sdcard");
        }

        ret = display.init();
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: display");
        ESP_LOGI(TAG, "initialized: display");

        ret = touch.init(i2c);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: touch");
        ESP_LOGI(TAG, "initialized: touch");

        ret = lvgl.init();
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: lvgl");
        ESP_LOGI(TAG, "initialized: lvgl");

        return ESP_OK;

    err:
        deinit();

        return ret;
    }

    void Board::deinit() {
        sdcard.unmount();

        lvgl.deinit();
        touch.deinit();
        display.deinit();
        haptic.deinit();
        pmic.deinit();
        i2c.deinit();
        buttons.deinit();
    }

    void Board::poweroff() {
        haptic.vibrate(80);

        sdcard.unmount();

        lvgl.deinit();
        touch.deinit();
        display.deinit();
        haptic.deinit();
        buttons.deinit();
        
        pmic.poweroff();
    }

    void Board::reboot() {
        haptic.vibrate(80);

        deinit();
        esp_restart();
    }
}
