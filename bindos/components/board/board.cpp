// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <board.h>
#include <esp_check.h>
#include <esp_lcd_panel_ops.h>
#include <esp_pm.h>
#include <esp_sleep.h>
#include <esp_system.h>
#include <init.h>

extern "C" {
    #include <axp2101.h>
}

namespace os {
    static const char *TAG = "bindos::board";

    constexpr auto CPU_MIN_FREQ = 40;
    constexpr auto CPU_MAX_FREQ = 240;

    esp_err_t Board::init(State &state) {
        esp_err_t ret = ESP_OK;

        {
            esp_pm_config_t config = {};
            config.min_freq_mhz = CPU_MIN_FREQ,
            config.max_freq_mhz = CPU_MAX_FREQ,
            ret = esp_pm_configure(&config);

            ESP_RETURN_ON_ERROR(ret, TAG, "failed to configure: power management");
        }

        ret = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable all sleep wakeup source");

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

        display.set_brightness(state.brightness);
        os::logo();

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

    esp_err_t Board::suspend() {
        esp_err_t ret = ESP_OK;

        ret = lvgl.suspend();
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to suspend: lvgl");

        ret = touch.sleep(true);
        ESP_GOTO_ON_ERROR(ret, resume_lvgl, TAG, "failed to suspend: touch");

        ret = esp_lcd_panel_disp_sleep(display.handle(), true);
        ESP_GOTO_ON_ERROR(ret, resume_touch, TAG, "failed to suspend: display");

        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO2, false);
        ESP_GOTO_ON_ERROR(ret, resume_display, TAG, "failed to turn off display ldo");

        return ESP_OK;

    resume_display:
        esp_lcd_panel_disp_sleep(display.handle(), false);

    resume_touch:
        touch.sleep(false);

    resume_lvgl:
        lvgl.resume();

        return ret;
    }

    esp_err_t Board::resume() {
        esp_err_t ret = ESP_OK;

        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO2, true);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to turn on display ldo");

        ret = esp_lcd_panel_disp_sleep(display.handle(), false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to resume: display");

        ret = touch.sleep(false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to resume: touch");

        ret = lvgl.resume();
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to resume: lvgl");

        return ESP_OK;
    }
}
