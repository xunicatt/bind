// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/i2c.h>

namespace bsp {
    esp_err_t I2c::init() {
        if (handle_ != nullptr)  {
            return ESP_OK;
        }

        const i2c_master_bus_config_t config = {
            .i2c_port = I2c::PORT,
            .sda_io_num = I2c::SDA,
            .scl_io_num = I2c::SCL,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 0,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = true,
                .allow_pd = false,
            },
        };
        return i2c_new_master_bus(&config, &handle_);
    }

    void I2c::deinit() {
        if (handle_ != nullptr) {
            i2c_del_master_bus(handle_);
            handle_ = nullptr;
        }
    }

    I2c::~I2c() {
        deinit();
    }

    i2c_master_bus_handle_t I2c::handle() {
        return handle_;
    }
}
