// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/haptic.h>
#include <esp_check.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" {
    #include <axp2101.h>
}

namespace bsp {
    static const char *TAG = "bsp::haptic";

    esp_err_t Haptic::init() {
        esp_err_t ret = ESP_OK;

        if (timer_ != nullptr) {
            return ESP_OK;
        }
        
        const gpio_config_t config = {
            .pin_bit_mask = BIT64(HAPTIC),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ret = gpio_config(&config);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to configure pin");

        const esp_timer_create_args_t args = {
            .callback = [](void *arg) {
                Haptic *haptic = static_cast<Haptic*>(arg);
                haptic->off();
            },
            .arg = static_cast<void*>(this),
            .dispatch_method = ESP_TIMER_TASK,
            .name = "HAPTIC",
            .skip_unhandled_events = false,
        };
        ret = esp_timer_create(&args, &timer_);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to configure pin");

        return ESP_OK;

    err:
        gpio_reset_pin(HAPTIC);
        return ret;
    }

    void Haptic::deinit() {
        if (timer_ != nullptr) {
            esp_timer_delete(timer_);
            timer_ = nullptr;
        }

        gpio_reset_pin(HAPTIC);
    }

    Haptic::~Haptic() {
        deinit();
    }

    void Haptic::on() {
        esp_err_t ret = ESP_OK;

        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO3, true);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "failed to enable haptic ldo: %s", esp_err_to_name(ret));
            return;
        }

        ret = gpio_set_level(HAPTIC, 1);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to set gpio level");

        return;

    err:
        axp2101_set_ldo_enabled(AXP2101_LDO_ALDO3, false);
    }

    void Haptic::off() {
        esp_err_t ret = ESP_OK;

        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO3, false);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "failed to disable haptic ldo: %s", esp_err_to_name(ret));
            return;
        }

        ret = gpio_set_level(HAPTIC, 0);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "failed to set gpio level: %s", esp_err_to_name(ret));
            return;
        }
    }

    void Haptic::vibrate(uint32_t ms) {
        on();
        esp_timer_start_once(timer_, ms * 1000);
    }

    void Haptic::set_strength(uint8_t strength) {
        esp_err_t ret = ESP_OK;
        const float voltage = 1.5 + (strength * 0.015);

        ret = axp2101_set_ldo_voltage(AXP2101_LDO_ALDO3, voltage);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "failed to set ldo voltage: %s", esp_err_to_name(ret));
        }
    }
}
