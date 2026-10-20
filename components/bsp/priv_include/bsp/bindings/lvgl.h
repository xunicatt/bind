// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BSP_BINDINGS_LVGL_H
#define BSP_BINDINGS_LVGL_H

#include <esp_lcd_panel_dev.h>
#include <lvgl.h>

namespace bsp {
    namespace lvgl {
        static constexpr auto BUFFER_HEIGHT = 128;

        bool flush_ready_cb(esp_lcd_panel_io_handle_t io,
                            esp_lcd_panel_io_event_data_t *event, void *data);
        void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *color);
        void invalidate_area_cb(lv_event_t *event);
        void touch_input_cb(lv_indev_t *indev, lv_indev_data_t *data);
    }
}

#endif
