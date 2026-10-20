// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/buttons.h>
#include <button_gpio.h>
#include <esp_check.h>

namespace bsp {
    static const char *TAG = "bsp::buttons";

    esp_err_t Buttons::init() {
        if (boot_ != nullptr && pwr_ != nullptr) {
            return ESP_OK;
        }

        esp_err_t ret = ESP_OK;

        const button_config_t config = {};
        button_gpio_config_t gpio_config = {
            .gpio_num = PWR_BUTTON,
            .active_level = 1,
            .enable_power_save = true,
            .disable_pull = false,
        };
        esp_log_level_set("gpio", ESP_LOG_NONE);

        ret = iot_button_new_gpio_device(&config, &gpio_config, &pwr_);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to add power button");

        gpio_config.gpio_num = BOOT_BUTTON;
        gpio_config.active_level = 0;
        ret = iot_button_new_gpio_device(&config, &gpio_config, &boot_);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to add boot button");

        esp_log_level_set("gpio", ESP_LOG_INFO);

        return ESP_OK;

    err:
        iot_button_delete(pwr_);

        return ret;
    }

    void Buttons::deinit() {
        if (pwr_ != nullptr) {
            iot_button_delete(pwr_);
            pwr_ = nullptr;
        }

        if (boot_ != nullptr) {
            iot_button_delete(boot_);
            boot_ = nullptr;
        }
    }

    Buttons::~Buttons() {
        deinit();
    }

    button_handle_t Buttons::pwr() {
        return pwr_;
    }

    button_handle_t Buttons::boot() {
        return boot_;
    }
}
