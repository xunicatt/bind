// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/audio.h>
#include <esp_check.h>
#include <esp_codec_dev.h>

namespace bsp {
    static const char *TAG = "bsp::audio";

    esp_err_t Microphone::init(I2c& i2c, I2s& i2s) {
        if (handle_ != nullptr) {
            return ESP_OK;
        }

        esp_err_t ret = ESP_OK;

        {
            audio_codec_i2c_cfg_t config = {
                .port = I2c::PORT,
                .addr = I2C_ADDR,
                .bus_handle = i2c.handle(),
                .clock_speed_hz = 0,
            };
            ctrl_ = audio_codec_new_i2c_ctrl(&config);
            ESP_RETURN_ON_FALSE(ctrl_ != nullptr, ESP_FAIL, TAG, "failed to create ctrl interface");
        }

        {
            audio_codec_i2s_cfg_t config = {
                .port = I2s::PORT,
                .rx_handle = i2s.rx(),
                .tx_handle = nullptr,
                .clk_src = 0,
            };
            data_ = audio_codec_new_i2s_data(&config);
            ESP_GOTO_ON_FALSE(data_ != nullptr, ESP_FAIL, err, TAG, "failed to create data interface");
        }

        {
            es7210_codec_cfg_t config = {
                .ctrl_if      = ctrl_,
                .master_mode  = false,
                .mic_selected = ES7210_SEL_MIC1 | ES7210_SEL_MIC2,
                .mclk_src     = ES7210_MCLK_FROM_PAD,
                .mclk_div     = I2S_MCLK_MULTIPLE_256,
            };
            codec_ = es7210_codec_new(&config);
            ESP_GOTO_ON_FALSE(codec_ != nullptr, ESP_FAIL, err, TAG, "failed to create codec interface");
        }

        {
            esp_codec_dev_cfg_t config = {
                .dev_type = ESP_CODEC_DEV_TYPE_IN,
                .codec_if = codec_,
                .data_if  = data_,
            };
            handle_ = esp_codec_dev_new(&config);
            ESP_GOTO_ON_FALSE(handle_ != nullptr, ESP_FAIL, err, TAG, "failed to create codec device");
        }

        ret = esp_codec_set_disable_when_closed(handle_, true);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to set disable codec on close (es7210)");

        return ESP_OK;

    err:
        deinit();

        return ret;
    }

    void Microphone::deinit() {
        if (handle_ != nullptr) {
            esp_codec_dev_delete(handle_);
            handle_ = nullptr;
        }

        if (codec_ != nullptr) {
            audio_codec_delete_codec_if(codec_);
            codec_ = nullptr;
        }

        if (data_ != nullptr) {
            audio_codec_delete_data_if(data_);
            data_ = nullptr;
        }

        if (ctrl_ != nullptr) {
            audio_codec_delete_ctrl_if(ctrl_);
            ctrl_ = nullptr;
        }
    }

    Microphone::~Microphone() {
        deinit();
    }

    esp_codec_dev_handle_t Microphone::handle() {
        return handle_;
    }

    const esp_codec_dev_sample_info_t& Microphone::sample() {
        return sample_;
    }

    esp_err_t Speaker::init(I2c& i2c, I2s& i2s) {
        esp_err_t ret = ESP_OK;

        {
            audio_codec_i2c_cfg_t config = {
                .port = I2c::PORT,
                .addr = I2C_ADDR,
                .bus_handle = i2c.handle(),
                .clock_speed_hz = 0,
            };
            ctrl_ = audio_codec_new_i2c_ctrl(&config);
            ESP_RETURN_ON_FALSE(ctrl_ != nullptr, ESP_FAIL, TAG, "failed to create ctrl interface");
        }

        {
            audio_codec_i2s_cfg_t config = {
                .port = I2s::PORT,
                .rx_handle = nullptr,
                .tx_handle = i2s.tx(),
                .clk_src = 0,
            };
            data_ = audio_codec_new_i2s_data(&config);
            ESP_GOTO_ON_FALSE(data_ != nullptr, ESP_FAIL, err, TAG, "failed to create data interface");
        }

        gpio_ = audio_codec_new_gpio();
        ESP_GOTO_ON_FALSE(gpio_ != nullptr, ESP_FAIL, err, TAG, "failed to create gpio interface");

        {
            es8311_codec_cfg_t config = {
                .ctrl_if = ctrl_,
                .gpio_if = gpio_,
                .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
                .pa_pin = PWR_AMP,
                .pa_reverted = false,
                .master_mode = false,
                .use_mclk = true,
                .digital_mic = false,
                .invert_mclk = false,
                .invert_sclk = false,
                .hw_gain = {
                    .pa_voltage = 3.3,
                    .codec_dac_voltage = 3.3,
                    .pa_gain = 0,
                },
                .no_dac_ref = false,
                .mclk_div = I2S_MCLK_MULTIPLE_256,
            };
            codec_ = es8311_codec_new(&config);
            ESP_GOTO_ON_FALSE(codec_ != nullptr, ESP_FAIL, err, TAG, "failed to create codec interface");
        }

        {
            esp_codec_dev_cfg_t config = {
                .dev_type = ESP_CODEC_DEV_TYPE_OUT,
                .codec_if = codec_,
                .data_if = data_,
            };
            handle_ = esp_codec_dev_new(&config);
            ESP_GOTO_ON_FALSE(handle_ != nullptr, ESP_FAIL, err, TAG, "failed to create codec device");
        }

        ret = esp_codec_set_disable_when_closed(handle_, true);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to set disable codec on close (es8311)");
       
        return ESP_OK;

    err:
        deinit();

        return ret;
    }

    void Speaker::deinit() {
        if (handle_ != nullptr) {
            esp_codec_dev_delete(handle_);
            handle_ = nullptr;
        }

        if (codec_ != nullptr) {
            audio_codec_delete_codec_if(codec_);
            codec_ = nullptr;
        }

        if (gpio_ != nullptr) {
            audio_codec_delete_gpio_if(gpio_);
            gpio_ = nullptr;
        }

        if (data_ != nullptr) {
            audio_codec_delete_data_if(data_);
            data_ = nullptr;
        }

        if (ctrl_ != nullptr) {
            audio_codec_delete_ctrl_if(ctrl_);
            ctrl_ = nullptr;
        }
    }

    Speaker::~Speaker() {
        deinit();
    }

    esp_codec_dev_handle_t Speaker::handle() {
        return handle_;
    }

    const esp_codec_dev_sample_info_t& Speaker::sample() {
        return sample_;
    }
}
