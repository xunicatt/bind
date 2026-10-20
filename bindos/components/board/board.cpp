// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <board.h>
#include <esp_check.h>
#include <esp_system.h>

namespace os {
    static const char *TAG = "bindos::board";

    esp_err_t Board::init() {
        esp_err_t ret = ESP_OK;

        ret = ramfs.mount();
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to mount: ramfs");
        ESP_LOGI(TAG, "mounted: ramfs");

        ret = storage.mount();
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to mount: storage");
        ESP_LOGI(TAG, "mounted: storage");

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

        ret = rtc.init(i2c);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: rtc");
        ESP_LOGI(TAG, "initialized: rtc");

        ret = imu.init(i2c);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: imu");
        ESP_LOGI(TAG, "initialized: imu");

        ret = i2s.init();
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: i2s");
        ESP_LOGI(TAG, "initialized: i2s");
        
        ret = microphone.init(i2c, i2s);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: microphone");
        ESP_LOGI(TAG, "initialized: microphone");

        ret = speaker.init(i2c, i2s);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize: speaker");
        ESP_LOGI(TAG, "initialized: speaker");

        return ESP_OK;

    err:
        deinit();

        return ret;
    }

    void Board::deinit() {
        ramfs.unmount();
        storage.unmount();
        sdcard.unmount();

        speaker.deinit();
        microphone.deinit();
        i2s.deinit();
        imu.deinit();
        rtc.deinit();
        lvgl.deinit();
        touch.deinit();
        display.deinit();
        haptic.deinit();
        pmic.deinit();
        i2c.deinit();
        buttons.deinit();
    }

    void Board::poweroff() {
        haptic.on();
        vTaskDelay(pdMS_TO_TICKS(80));
        haptic.off();

        ramfs.unmount();
        storage.unmount();
        sdcard.unmount();

        speaker.deinit();
        microphone.deinit();
        i2s.deinit();
        imu.deinit();
        rtc.deinit();
        lvgl.deinit();
        touch.deinit();
        display.deinit();
        haptic.deinit();
        buttons.deinit();
        
        pmic.poweroff();
    }

    void Board::reboot() {
        haptic.on();
        vTaskDelay(pdMS_TO_TICKS(80));
        haptic.off();

        deinit();
        esp_restart();
    }
}
