// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <board.h>
#include <esp_check.h>
#include <esp_sleep.h>
#include <fonts.h>
#include <init.h>
#include <state.h>

#define OS_VERSION "v0.1"

namespace os {
    static const char *TAG = "bindos::init";
    static TaskHandle_t sleep_task = nullptr;
    static Board board;
    static State state;

    static void osinfo() {
        ESP_LOGI(TAG, "*****************************");
        ESP_LOGI(TAG, "*  Starting BindOS(%s)..  *", OS_VERSION);
        ESP_LOGI(TAG, "*****************************");
    }

    static void sleep_handler_task(void *arg) {
        ESP_UNUSED(arg);

        esp_err_t ret = ESP_OK;

        while (true) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            vTaskDelay(pdMS_TO_TICKS(50));

            ret = board.suspend();
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "failed to suspend: %s", esp_err_to_name(ret));
                board.resume();
                continue;
            }

            state.sleep = true;

            esp_sleep_enable_gpio_wakeup();
            esp_light_sleep_start();

            ret = board.resume();
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "failed to resume: %s", esp_err_to_name(ret));
            }

            vTaskDelay(pdMS_TO_TICKS(500));
            ulTaskNotifyTake(pdTRUE, 0);

            state.sleep = false;
        }
    }

    static void power_button_single_click_handler(void *arg, void *user_data) {
        if (sleep_task != nullptr && !state.sleep) {
            xTaskNotifyGive(sleep_task);
        }
    }

    static void power_button_long_press_handler(void *arg, void *user_data) {
        poweroff();
    }

    esp_err_t init() {
        esp_err_t ret = ESP_OK;

        osinfo();

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

    esp_err_t run() {
        xTaskCreate(
            sleep_handler_task, "sleep_handler_task",
            4 * 1024,
            nullptr, 5,
            &sleep_task
        );

        iot_button_register_cb(
            board.buttons.pwr(), BUTTON_SINGLE_CLICK,
            nullptr, power_button_single_click_handler,
            nullptr 
        );

        iot_button_register_cb(
            board.buttons.pwr(), BUTTON_LONG_PRESS_START,
            nullptr, power_button_long_press_handler,
            nullptr 
        );

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
