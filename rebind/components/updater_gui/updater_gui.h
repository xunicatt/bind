// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef UPDATER_GUI_H
#define UPDATER_GUI_H

#include <board.h>
#include <lvgl.h>
#include <updater.h>

namespace rebind {
    class UpdaterGui {
    public:
        UpdaterGui(Board& board, Updater& updater);

        void init();
        void deinit();
        void notify();
    
    private:
        void build_header();
        void build_body();
        void build_footer();
        void show_sdcard_missing();
        void show_update_check();
        void show_update_progress();
        void show_update_unavailable();
        void show_update_complete(const char *text_msg, const char* button_label);

        static void progress_timer_cb(lv_timer_t *timer);
        static void power_button_cb(lv_event_t *event);
        static void restart_button_cb(lv_event_t *event);
        static void update_button_cb(lv_event_t *event);
        static void update_task(void *arg);

        Board& board_;
        Updater& updater_;

        lv_obj_t *parent_ = nullptr;
        lv_timer_t *timer_ = nullptr;

        lv_obj_t *header_ = nullptr;
        lv_obj_t *header_title_ = nullptr;

        lv_obj_t *content_ = nullptr;
        lv_obj_t *content_wrapper_ = nullptr;
        lv_obj_t *content_spinner_ = nullptr;
        lv_obj_t *content_bar_ = nullptr;
        lv_obj_t *content_icon_ = nullptr;
        lv_obj_t *content_label1_ = nullptr;
        lv_obj_t *content_label2_ = nullptr;
        lv_obj_t *content_button_ = nullptr;

        lv_obj_t *footer_ = nullptr;
        lv_obj_t *footer_power_button_ = nullptr;
        lv_obj_t *footer_power_label_ = nullptr;
        lv_obj_t *footer_restart_button_ = nullptr;
        lv_obj_t *footer_restart_label_ = nullptr;
    };
}

#endif
