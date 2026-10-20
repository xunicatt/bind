// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_LVGL_H
#define BSP_LVGL_H

#include <bsp/display.h>
#include <esp_err.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lvgl.h>
#include <sdkconfig.h>

namespace bsp {
    class Lvgl {
    public:
        Lvgl(const Lvgl&) = delete;
        Lvgl& operator=(const Lvgl&) = delete;

        Lvgl() = default;
        ~Lvgl();

        esp_err_t init();
        void deinit();
        esp_err_t suspend();
        esp_err_t resume();

        lv_display_t *display();
        lv_indev_t *indev();

    private:
        lv_display_t *display_ = nullptr;
        lv_indev_t *indev_ = nullptr;
        uint8_t *buffer1_ = nullptr;
        uint8_t *buffer2_ = nullptr;

        TaskHandle_t task_ = nullptr;
        esp_timer_create_args_t args_;
        esp_timer_handle_t timer_ = nullptr;

        lv_fs_drv_t storage_;
        lv_fs_drv_t sdcard_;

        bool run_ = true;

        static constexpr auto TASK_PRIORITY = 2;
        static constexpr auto TASK_STACK_SIZE = (8 * 1024);

        friend class LvglTask;
    };
}

#endif
