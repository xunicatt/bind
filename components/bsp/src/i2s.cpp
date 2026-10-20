// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/i2s.h>
#include <esp_check.h>

namespace bsp {
    static const char *TAG = "bsp::i2s";

    esp_err_t I2s::init() {
        if (rx_ != nullptr && tx_ != nullptr) {
            return ESP_OK;
        }

        esp_err_t ret = ESP_OK;

        {
            i2s_chan_config_t config = I2S_CHANNEL_DEFAULT_CONFIG(
                I2s::PORT,
                I2S_ROLE_MASTER
            );
            config.auto_clear = true;
            ret = i2s_new_channel(&config, &tx_, &rx_);
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to create i2s channels");
        }

        {
            const i2s_std_config_t config = {
                .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2s::AUDIO_SAMPLE_RATE),
                .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                    I2S_DATA_BIT_WIDTH_16BIT,
                    I2S_SLOT_MODE_STEREO
                ),
                .gpio_cfg = {
                    .mclk = I2s::MCLK,
                    .bclk = I2s::BCLK,
                    .ws   = I2s::WS,
                    .dout = I2s::DOUT,
                    .din  = I2s::DIN,
                    .invert_flags = {
                        .mclk_inv = false,
                        .bclk_inv = false,
                        .ws_inv = false,
                    },
                },
            };
            ret = i2s_channel_init_std_mode(tx_, &config);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize i2s tx channel");
            ret = i2s_channel_enable(tx_);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize i2s tx channel");

            ret = i2s_channel_init_std_mode(rx_, &config);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize i2s rx channel");
            ret = i2s_channel_enable(rx_);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize i2s rx channel");
        }

        return ESP_OK;

    err:
        deinit();

        return ret;
    }

    void I2s::deinit() {
        if (rx_ != nullptr) {
            i2s_channel_disable(rx_);
            i2s_del_channel(rx_);
            rx_ = nullptr;
        }

        if (tx_ != nullptr) {
            i2s_channel_disable(tx_);
            i2s_del_channel(tx_);
            tx_ = nullptr;
        }
    }

    I2s::~I2s() {
        deinit();
    }

    i2s_chan_handle_t I2s::rx() {
        return rx_;
    }

    i2s_chan_handle_t I2s::tx() {
        return tx_;
    }
}
