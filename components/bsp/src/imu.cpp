// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/imu.h>
#include <esp_check.h>

namespace bsp {
    static const char *TAG = "bsp::imu";

    esp_err_t Imu::init(I2c& i2c) {
        if (dev_.dev_handle != nullptr) {
            return ESP_OK;
        }

        esp_err_t ret = ESP_OK;

        ret = qmi8658_init(&dev_, i2c.handle(), I2C_ADDR);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize");

        ret = qmi8658_enable_accel(&dev_, false);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to disable accelerometer");

        ret = qmi8658_enable_gyro(&dev_, false);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to disable gyroscope ");

        qmi8658_set_accel_unit_mps2(&dev_, true);
        qmi8658_set_gyro_unit_rads(&dev_, true);
        qmi8658_set_display_precision(&dev_, 4);

        return ESP_OK;

    err:
        deinit();

        return ret;
    }

    void Imu::deinit() {
        if (dev_.dev_handle != nullptr) {
            i2c_master_bus_rm_device(dev_.dev_handle);
            dev_.dev_handle = nullptr;
        }
    }

    Imu::~Imu() {
        deinit();
    }

    qmi8658_dev_t *Imu::dev() {
        if (dev_.dev_handle != nullptr) {
            return &dev_;
        }

        return nullptr;
    }
}
