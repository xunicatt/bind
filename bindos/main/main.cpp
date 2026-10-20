// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <board.h>
#include <esp_check.h>
#include <esp_err.h>
#include <fonts.h>

static const char *TAG = "bindos::main";
static os::Board board = {};

extern "C" int app_main(void) {
    esp_err_t ret = ESP_OK;

    ret = board.init();
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize: board");

    board.display.set_brightness(100);

    ret = iot_button_register_cb(
        board.buttons.boot(),
        BUTTON_LONG_PRESS_START,
        nullptr,
        [](void *arg, void *user_data) {
            board.poweroff();
        },
        nullptr 
    );
    if (ret != ESP_OK) {
        ESP_LOGE(
            TAG, "failed to register button cb: %s", esp_err_to_name(ret)
        );
        ret = ESP_OK;
    }

    // DEMO
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

    return ESP_OK;
}

