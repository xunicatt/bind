// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/bindings/lvgl.h>
#include <bsp/display.h>
#include <esp_check.h>
#include <esp_lcd_co5300.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

namespace bsp {
    static const char *TAG = "bsp::display";
    static SemaphoreHandle_t s_sem = nullptr;
    static esp_lcd_panel_handle_t s_panel = nullptr;

    constexpr auto MAX_TRANSFER_SIZE = (
        Display::WIDTH * lvgl::BUFFER_HEIGHT * Display::BPP
    );

    extern "C" {
        static void IRAM_ATTR display_sync_isr(void *arg) {
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

    esp_err_t Display::init() {
        esp_err_t ret = ESP_OK;

        if (panel_ != nullptr && io_ != nullptr) {
            return ESP_OK;
        }

        sem_ = xSemaphoreCreateBinary();
        ESP_RETURN_ON_FALSE(sem_ != nullptr, ESP_FAIL, TAG, "failed tp create semaphore");
        s_sem = sem_;

        {
            const gpio_config_t config = {
                .pin_bit_mask = BIT64(TE),
                .mode = GPIO_MODE_INPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_NEGEDGE,
            };
            ret = gpio_config(&config);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to setup gpio");

            ret = gpio_isr_handler_add(TE, display_sync_isr, nullptr);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to add isr");
        }

        {
            const spi_bus_config_t config = {
                .data0_io_num = SIO0,
                .data1_io_num = SIO1,
                .sclk_io_num = SCLK,
                .data2_io_num = SIO2,
                .data3_io_num = SIO3,
                .data4_io_num = GPIO_NUM_NC,
                .data5_io_num = GPIO_NUM_NC,
                .data6_io_num = GPIO_NUM_NC,
                .data7_io_num = GPIO_NUM_NC,
                .data_io_default_level = false,
                .max_transfer_sz = MAX_TRANSFER_SIZE,
                .flags = 0,
                .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
                .intr_flags = 0,
            };
            ret = spi_bus_initialize(SPI, &config, SPI_DMA_CH_AUTO);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize spi bus");
        }

        {
            const esp_lcd_panel_io_spi_config_t config = {
                .cs_gpio_num = CS,
                .dc_gpio_num = GPIO_NUM_NC,
                .spi_mode = 0,
                .pclk_hz = SPI_MASTER_FREQ,
                .trans_queue_depth = 6,
                .on_color_trans_done = lvgl::flush_ready_cb,
                .user_ctx = nullptr,
                .lcd_cmd_bits = 32,
                .lcd_param_bits = 8,
                .cs_ena_pretrans = 0,
                .cs_ena_posttrans = 0,
                .flags = { 
                    .dc_high_on_cmd = false,
                    .dc_low_on_data = false,
                    .dc_low_on_param = false,
                    .octal_mode = false,
                    .quad_mode = true,
                    .sio_mode = false,
                    .psram_dma_direct = false,
                    .lsb_first = false,
                    .cs_high_active = false,
                },
            };
            ret = esp_lcd_new_panel_io_spi(SPI, &config, &io_);
            ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to create panel io");
        }

        {
            co5300_vendor_config_t vendor_config = {};
            vendor_config.flags.use_qspi_interface = true;

            const esp_lcd_panel_dev_config_t config = {
                .reset_gpio_num = RST,
                .color_space = ESP_LCD_COLOR_SPACE_RGB,
                .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
                .bits_per_pixel = BPP,
                .flags = {
                    .reset_active_high = false,
                },
                .vendor_config = static_cast<void*>(&vendor_config),
            };
            ret = esp_lcd_new_panel_co5300(io_, &config, &panel_);
            ESP_RETURN_ON_ERROR(ret, TAG, "failed to create panel");
        }

        ret = esp_lcd_panel_reset(panel_);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to reset panel");
        ret = esp_lcd_panel_init(panel_);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to initialize panel");
        ret = esp_lcd_panel_set_gap(panel_, X_OFFSET, Y_OFFSET);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to set panel gap");
        ret = esp_lcd_panel_disp_on_off(panel_, true);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to turn panel on");
        ret = esp_lcd_panel_co5300_set_brightness(panel_, brightness_);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to set brightness");

        s_panel = panel_;

        return ESP_OK;

    err:
        deinit();

        return ret;
    }

    void Display::deinit() {
        if (panel_ != nullptr) {
            esp_lcd_panel_del(panel_);
            panel_ = nullptr;
            s_panel = nullptr;
        }

        if (io_ != nullptr) {
            esp_lcd_panel_io_del(io_);
            io_ = nullptr;
        }

        spi_bus_free(SPI);

        gpio_isr_handler_remove(TE);
        gpio_reset_pin(TE);

        if (sem_ != nullptr) {
            vSemaphoreDelete(sem_);
            sem_ = nullptr;
            s_sem = nullptr;
        }
    }

    Display::~Display() {
        deinit();
    }

    esp_lcd_panel_handle_t Display::handle() {
        return panel_;
    }

    void Display::set_brightness(uint8_t percentage) {
        esp_err_t ret = esp_lcd_panel_co5300_set_brightness(panel_, percentage);

        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "failed to set display brightness: %s", esp_err_to_name(ret));
        } else {
            brightness_ = percentage;
        }
    }

    namespace lvgl {
        void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *color) {
            static bool first = true;

            if (s_sem == nullptr || s_panel == nullptr) {
                return;
            }

            if (first) {
                xSemaphoreTake(s_sem, 0);
                if (xSemaphoreTake(s_sem, pdMS_TO_TICKS(64)) == pdFALSE) {
                    ESP_LOGW(TAG, "TE wait timed out, flushing without sync");
                }

                first = false;
            }

            (void)esp_lcd_panel_draw_bitmap(
                s_panel,
                area->x1, area->y1,
                area->x2 + 1, area->y2 + 1,
                color
            );

            if (lv_display_flush_is_last(display)) {
                first = true;
            }
        }

        void invalidate_area_cb(lv_event_t *event) {
            lv_area_t *area = (lv_area_t*)lv_event_get_param(event);
            const uint16_t x1 = area->x1,
                           x2 = area->x2,
                           y1 = area->y1,
                           y2 = area->y2;

            area->x1 = x1 & ~1;
            area->y1 = y1 & ~1;
            area->x2 = (x2 & ~1) + 1;
            area->y2 = (y2 & ~1) + 1;
        }
    }
}
