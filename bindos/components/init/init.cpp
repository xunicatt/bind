// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <board.h>
#include <esp_check.h>
#include <fonts.h>
#include <init.h>
#include <state.h>

namespace os {
    static const char *TAG = "bindos::init";
    static Board board;
    static State state;

    esp_err_t init() {
        esp_err_t ret = ESP_OK;

        ret = state.init();
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize: state");
        ESP_LOGI(TAG, "initialized: state");

        ret = state.load();
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to load: state");
        ESP_LOGI(TAG, "loaded: state");

        ret = board.init(state);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialze: board");
        ESP_LOGI(TAG, "initialized: board");

        return ESP_OK;
    }

    void logo() {
        lv_lock();
        {
            lv_obj_t *parent = lv_display_get_screen_active(
                board.lvgl.display()
            );

            lv_obj_t *text = lv_label_create(parent);
            lv_label_set_text(text, "BindOS");
            lv_obj_set_style_text_color(text, lv_color_hex(0xffffff), LV_PART_MAIN);
            lv_obj_set_style_text_font(text, &geo_regular_48, LV_PART_MAIN);
            lv_obj_center(text);
        }
        lv_unlock();
    }

    void poweroff() {
        ESP_LOGI(TAG, "shutdown initiated!");
        state.save();
        state.deinit();
        board.poweroff();
    }

    void reboot() {
        ESP_LOGI(TAG, "reboot initiated!");
        state.save();
        state.deinit();
        board.reboot();
    }
}
