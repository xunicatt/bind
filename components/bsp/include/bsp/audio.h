// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_AUDIO_H
#define BSP_AUDIO_H

#include <bsp/i2c.h>
#include <bsp/i2s.h>
#include <driver/gpio.h>
#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>
#include <esp_err.h>

namespace bsp {
    class Microphone {
    public:
        Microphone(const Microphone&) = delete;
        Microphone& operator=(const Microphone&) = delete;

        Microphone() = default;
        ~Microphone();

        esp_err_t init(I2c& i2c, I2s& i2s);
        void deinit();
        esp_codec_dev_handle_t handle();
        const esp_codec_dev_sample_info_t& sample();

    private:
        esp_codec_dev_handle_t handle_ = nullptr;
        const audio_codec_if_t *codec_ = nullptr;
        const audio_codec_ctrl_if_t *ctrl_ = nullptr;
        const audio_codec_data_if_t *data_ = nullptr;
        const esp_codec_dev_sample_info_t sample_= {
            .bits_per_sample = I2S_DATA_BIT_WIDTH_16BIT,
            .channel = 2,
            .channel_mask = ES7210_SEL_MIC1 | ES7210_SEL_MIC2,
            .sample_rate = I2s::AUDIO_SAMPLE_RATE,
            .mclk_multiple = 0,
        };

        static constexpr uint8_t I2C_ADDR = 0x80;
    };

    class Speaker {
    public:
        Speaker(const Speaker&) = delete;
        Speaker& operator=(const Speaker&) = delete;

        Speaker() = default;
        ~Speaker();

        esp_err_t init(I2c& i2c, I2s& i2s);
        void deinit();
        esp_codec_dev_handle_t handle();
        const esp_codec_dev_sample_info_t& sample();

    private:
        esp_codec_dev_handle_t handle_ = nullptr;
        const audio_codec_if_t *codec_ = nullptr;
        const audio_codec_ctrl_if_t *ctrl_ = nullptr;
        const audio_codec_data_if_t *data_ = nullptr;
        const audio_codec_gpio_if_t *gpio_ = nullptr;
        const esp_codec_dev_sample_info_t sample_ = {
            .bits_per_sample = I2S_DATA_BIT_WIDTH_16BIT,
            .channel = 2,
            .channel_mask = 0,
            .sample_rate = I2s::AUDIO_SAMPLE_RATE,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        };

        static constexpr uint8_t I2C_ADDR = 0x30;
        static constexpr auto PWR_AMP = GPIO_NUM_46;
    };
}

#endif
