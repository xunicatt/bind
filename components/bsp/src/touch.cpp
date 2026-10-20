// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/bindings/lvgl.h>
#include <bsp/display.h>
#include <bsp/touch.h>
#include <esp_check.h>
#include <esp_lcd_touch_ft5x06.h>

namespace bsp {
    static const char *TAG = "bsp::touch";
    static SemaphoreHandle_t s_sem = nullptr;
    static esp_lcd_touch_handle_t s_handle = nullptr;

    extern "C" {
        static void IRAM_ATTR touch_interrupt_isr(void *arg) {
            ESP_UNUSED(arg);

            if (s_sem == nullptr) {
                return;
            }

            BaseType_t hp = pdFALSE;
            xSemaphoreGiveFromISR(s_sem, &hp);
            if (hp == pdTRUE) {
                portYIELD_FROM_ISR();
            }
        }
    }

    esp_err_t TouchInput::init(I2c& i2c) {
        esp_err_t ret = ESP_OK;

        if (handle_ != nullptr) {
            return ESP_OK;
        }

        sem_ = xSemaphoreCreateBinary();
        ESP_RETURN_ON_FALSE(
            sem_ != nullptr, ESP_FAIL,
            TAG, "failed to create semaphore"
        );
        s_sem = sem_;

        {
            const gpio_config_t config = {
                .pin_bit_mask = BIT64(INTR),
                .mode = GPIO_MODE_INPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_NEGEDGE,
            };
            ret = gpio_config(&config);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to setup interrupt pin");
        }

        ret = gpio_isr_handler_add(INTR, touch_interrupt_isr, nullptr);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed add interrupt isr");

        {
            const esp_lcd_panel_io_i2c_config_t config = {
                .dev_addr = I2C_ADDR,
                .on_color_trans_done = nullptr, 
                .user_ctx = nullptr,
                .control_phase_bytes = 1,
                .dc_bit_offset = 0,
                .lcd_cmd_bits = 8,
                .lcd_param_bits = 0,
                .flags = {
                    .dc_low_on_data = false,
                    .disable_control_phase = true,
                },
                .scl_speed_hz = (100 * 1000),
                .transaction_timeout_ms = 0,
            };
            ret = esp_lcd_new_panel_io_i2c(i2c.handle(), &config, &io_);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to create touch io");
        }

        {
            const esp_lcd_touch_config_t config = {
                .x_max = Display::WIDTH,
                .y_max = Display::HEIGHT,
                .rst_gpio_num = RST,
                .int_gpio_num = GPIO_NUM_NC,
                .levels = {
                    .reset = 0,
                    .interrupt = 0,
                },
                .flags = {
                    .swap_xy = false,
                    .mirror_x = false,
                    .mirror_y = false,
                },
                .process_coordinates = nullptr,
                .interrupt_callback = nullptr,
                .user_data = nullptr,
                .driver_data = nullptr,
            };
            ret = esp_lcd_touch_new_i2c_ft5x06(io_, &config, &handle_);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to create touch device");
        }

        s_handle = handle_;

        return ESP_OK;

    err:
        deinit();
        return ret;
    }

    void TouchInput::deinit() {
        if (handle_ != nullptr) {
            esp_lcd_touch_del(handle_);
            handle_ = nullptr;
            s_handle = nullptr;
        }

        if (io_ != nullptr) {
            esp_lcd_panel_io_del(io_);
            io_ = nullptr;
        }

        gpio_isr_handler_remove(INTR);
        gpio_reset_pin(INTR);

        if (sem_ != nullptr) {
            vSemaphoreDelete(sem_);
            sem_ = nullptr;
            s_sem = nullptr;
        }
    }

    TouchInput::~TouchInput() {
        deinit();
    }

    esp_err_t TouchInput::sleep(bool sleep) {
        if (handle_ == nullptr) {
            return ESP_ERR_INVALID_STATE;
        }

        esp_err_t ret = ESP_OK;
        const uint8_t pmode = sleep ? PMODE_HIBERNATE : PMODE_ACTIVE;

        if (sleep) {
            ret = reset();
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to reset");

            ret = esp_lcd_panel_io_tx_param(io_, PMODE_REG, &pmode, 1);
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to enter sleep");
        } else {
            ret = reset();
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to reset");

            ret = esp_lcd_panel_io_tx_param(io_, PMODE_REG, &pmode, 1);
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to exit sleep");
        }

        return ESP_OK;
    }

    esp_lcd_touch_handle_t TouchInput::handle() {
        return handle_;
    }

    namespace lvgl {
        void touch_input_cb(lv_indev_t *indev, lv_indev_data_t *data) {
            if (s_handle == nullptr || s_sem == nullptr) {
                return;
            }

            static esp_lcd_touch_point_data_t last_data = {};
            static bool was_pressed = false;

            esp_err_t ret = ESP_OK;

            ESP_UNUSED(indev);

            if (xSemaphoreTake(s_sem, 0) == pdTRUE) {
                (void)esp_lcd_touch_read_data(s_handle);

                uint8_t count = 0;
                esp_lcd_touch_point_data_t data = {};

                ret = esp_lcd_touch_get_data(s_handle, &data, &count, 1);
                if (ret != ESP_OK) {
                    ESP_LOGW(TAG, "failed to get touch data: %s", esp_err_to_name(ret));
                    return;
                }

                if (count > 0) {
                    last_data.x = data.x;
                    last_data.y = data.y;
                    was_pressed = true;
                } else {
                    was_pressed = false;
                }
            }

            data->state = was_pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
            data->point.x = last_data.x;
            data->point.y = last_data.y;
        }
    }
}
