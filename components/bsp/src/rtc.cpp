// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/rtc.h>
#include <esp_check.h>

namespace bsp {
    static const char *TAG = "bsp::rtc";

    esp_err_t Rtc::init(I2c& i2c) {
        if (dev_.dev_handle != nullptr) {
            return ESP_OK;
        }

        esp_err_t ret = ESP_OK;

        ret = pcf85063a_init(&dev_, i2c.handle(), I2C_ADDR);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize");

        return ESP_OK;
    }

    void Rtc::deinit() {
        if (dev_.dev_handle != nullptr) {
            i2c_master_bus_rm_device(dev_.dev_handle);
            dev_.dev_handle = nullptr;
        }
    }

    Rtc::~Rtc() {
        deinit();
    }

    pcf85063a_dev_t *Rtc::dev() {
        if (dev_.dev_handle != nullptr) {
            return &dev_;
        }

        return nullptr;
    }
}
