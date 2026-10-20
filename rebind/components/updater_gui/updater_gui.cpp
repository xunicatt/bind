// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <fonts.h>
#include <updater_gui.h>

namespace rebind {
    static const char *TAG = "rebind::gui";

    UpdaterGui::UpdaterGui(Board &board, Updater &updater)
        : board_(board), updater_(updater) {}

    void UpdaterGui::init()
    {
        if (parent_ != nullptr) {
            return;
        }

        lv_lock();
        {
            parent_ = lv_screen_active();
            lv_obj_set_style_pad_ver(parent_, 20, LV_PART_MAIN);
            lv_obj_set_flex_flow(parent_, LV_FLEX_FLOW_COLUMN);

            build_header();
            build_body();
            build_footer();
        }
        lv_unlock();
    }

    void UpdaterGui::deinit() {
        if (parent_ != nullptr) {
            lv_obj_delete(parent_);
            parent_ = nullptr;
        }

        if (timer_ != nullptr) {
            lv_timer_delete(timer_);
            timer_ = nullptr;
        }
    }

    void UpdaterGui::notify() {
        using Status = Updater::Status;

        if (parent_ == nullptr) {
            return;
        }

        switch (updater_.stauts()) {
        case Status::Updating:
            show_update_progress();
            break;

        case Status::UpdateUnavailable:
            show_update_unavailable();
            break;

        case Status::UpdateCompleted:
            show_update_complete("Update Completed.", "Re-Update");
            break;

        case Status::UpdateApplied:
            show_update_complete("Update is Already Applied.",  "Re-Update");
            break;

        case Status::UpdateFailed:
            show_update_complete("Failed to Update.\nPlease retry again.", "Retry");
            break;

        default:
            show_update_check();
            break;
        }
    }

    void UpdaterGui::build_header() {
        header_ = lv_obj_create(parent_);
        lv_obj_remove_style_all(header_);
        lv_obj_set_size(header_, lv_pct(100), 50);
        lv_obj_set_flex_flow(header_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(header_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollable(header_, false);
        
        header_title_ = lv_label_create(header_);
        lv_label_set_text(header_title_, "BindOS Recovery");
        lv_obj_set_style_text_font(header_title_, &geo_regular_32, LV_PART_MAIN);
        lv_obj_set_style_text_color(header_title_, lv_color_hex(0x8caaee), LV_PART_MAIN);
    }

    void UpdaterGui::build_body() {
        content_ = lv_obj_create(parent_);
        lv_obj_remove_style_all(content_);
        lv_obj_set_style_width(content_, lv_pct(100), LV_PART_MAIN);
        lv_obj_set_flex_flow(content_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(content_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollable(content_, false);
        lv_obj_set_style_pad_row(content_, 30, 0);
        lv_obj_set_flex_grow(content_, 1);

        if (board_.sdcard.mounted()) {
            show_update_check();
        } else {
            show_sdcard_missing();
        }
    }

    void UpdaterGui::build_footer() {
        footer_ = lv_obj_create(parent_);
        lv_obj_remove_style_all(footer_);
        lv_obj_set_style_pad_hor(footer_, 60, LV_PART_MAIN);
        lv_obj_set_size(footer_, lv_pct(100), 50);
        lv_obj_set_flex_flow(footer_, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(footer_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_scrollable(footer_, false);
        lv_obj_set_style_pad_column(footer_, 10, 0);

        footer_power_button_ = lv_button_create(footer_);
        lv_obj_add_event_cb(
            footer_power_button_, power_button_cb,
            LV_EVENT_CLICKED, static_cast<void*>(this)
        );
        lv_obj_set_style_bg_color(footer_power_button_, lv_color_hex(0xe78284), LV_PART_MAIN);
        lv_obj_set_flex_grow(footer_power_button_, 1);
        footer_power_label_ = lv_label_create(footer_power_button_);
        lv_label_set_text(footer_power_label_, LV_SYMBOL_POWER);
        lv_obj_set_style_text_font(footer_power_label_, &lv_font_montserrat_32, LV_PART_MAIN);
        lv_obj_center(footer_power_label_);

        footer_restart_button_ = lv_button_create(footer_);
        lv_obj_add_event_cb(
            footer_restart_button_, restart_button_cb,
            LV_EVENT_CLICKED, static_cast<void*>(this)
        );
        lv_obj_set_style_bg_color(footer_restart_button_, lv_color_hex(0xa6d189), LV_PART_MAIN);
        lv_obj_set_flex_grow(footer_restart_button_, 1);
        footer_restart_label_ = lv_label_create(footer_restart_button_);
        lv_label_set_text(footer_restart_label_, LV_SYMBOL_REFRESH);
        lv_obj_set_style_text_font(footer_restart_label_, &lv_font_montserrat_32, LV_PART_MAIN);
        lv_obj_center(footer_restart_label_);
    }

    void UpdaterGui::show_sdcard_missing() {
        lv_lock();
        {
            lv_obj_clean(content_);
            content_icon_ = lv_label_create(content_);
            lv_label_set_text(content_icon_, LV_SYMBOL_SD_CARD);
            lv_obj_set_style_text_font(content_icon_, &lv_font_montserrat_48, LV_PART_MAIN);

            content_label1_ = lv_label_create(content_);
            lv_label_set_text(content_label1_, "No SD Card Found!");
            lv_obj_set_style_text_font(content_label1_, &geo_regular_32, LV_PART_MAIN);
        }
        lv_unlock();
    }

    void UpdaterGui::show_update_check() {
        lv_lock();
        {
            lv_obj_clean(content_);
            content_spinner_ = lv_spinner_create(content_);
        }
        lv_unlock();
    }

    void UpdaterGui::show_update_progress() {
        lv_lock();
        {
            lv_obj_add_state(footer_power_button_, LV_STATE_DISABLED);
            lv_obj_add_state(footer_restart_button_, LV_STATE_DISABLED);

            lv_obj_clean(content_);
            content_icon_ = lv_label_create(content_);
            lv_label_set_text(content_icon_, LV_SYMBOL_UPLOAD);
            lv_obj_set_style_text_font(content_icon_, &lv_font_montserrat_48, LV_PART_MAIN);

            content_bar_ = lv_bar_create(content_);
            lv_obj_set_style_height(content_bar_, 40, LV_PART_MAIN);
            lv_bar_set_range(content_bar_, 0, 100);

            content_label1_ = lv_label_create(content_bar_);
            lv_label_set_text(content_label1_, "0%");
            lv_obj_set_style_text_color(content_label1_, lv_color_white(), LV_PART_MAIN);
            lv_obj_center(content_label1_);

            timer_ = lv_timer_create(
                progress_timer_cb, 100,
                static_cast<void*>(this)
            );
        }
        lv_unlock();
    }

    void UpdaterGui::show_update_unavailable() {
        lv_lock();
        {
            lv_obj_clean(content_);
            content_label1_ = lv_label_create(content_);
            lv_label_set_text(content_label1_, "No updates available.");
            lv_obj_set_style_text_font(content_label1_, &geo_regular_24, LV_PART_MAIN);
        }
        lv_unlock();
    }

    void UpdaterGui::show_update_complete(const char *text_msg, const char *button_label) {
        lv_lock();
        {
            lv_obj_remove_state(footer_power_button_, LV_STATE_DISABLED);
            lv_obj_remove_state(footer_restart_button_, LV_STATE_DISABLED);

            lv_obj_clean(content_);
            content_wrapper_ = lv_obj_create(content_);
            lv_obj_remove_style_all(content_wrapper_);
            lv_obj_set_style_width(content_wrapper_, lv_pct(100), LV_PART_MAIN);
            lv_obj_set_flex_grow(content_wrapper_, 1);
            content_label1_ = lv_label_create(content_wrapper_);
            lv_label_set_text(content_label1_, text_msg);
            lv_obj_set_style_text_align(content_label1_, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_font(content_label1_, &geo_regular_24, LV_PART_MAIN);
            lv_obj_center(content_label1_);
            
            content_button_ = lv_button_create(content_);
            lv_obj_add_event_cb(
                content_button_, update_button_cb,
                LV_EVENT_CLICKED, static_cast<void*>(this)
            );
            lv_obj_set_style_bg_color(content_button_, lv_color_hex(0x7287fd), LV_PART_MAIN);
            lv_obj_set_style_radius(content_button_, 5, LV_PART_MAIN);
            lv_obj_set_style_width(content_button_, lv_pct(70), LV_PART_MAIN);
            content_label2_ = lv_label_create(content_button_);
            lv_label_set_text(content_label2_, button_label);
            lv_obj_set_style_text_font(content_label2_, &geo_regular_32, LV_PART_MAIN);
            lv_obj_center(content_label2_);

            lv_timer_delete(timer_);
            timer_ = NULL;
        }
        lv_unlock();
    }

    void UpdaterGui::progress_timer_cb(lv_timer_t *timer) {
        void *arg = lv_timer_get_user_data(timer);
        auto *gui = static_cast<UpdaterGui*>(arg);

        uint8_t progress = gui->updater_.progress();

        lv_bar_set_value(gui->content_bar_, progress, LV_ANIM_OFF);
        lv_label_set_text_fmt(gui->content_label1_, "%d%%", progress);
    }

    void UpdaterGui::power_button_cb(lv_event_t *event) {
        void *arg = lv_event_get_user_data(event);
        auto *gui = static_cast<UpdaterGui*>(arg);

        gui->deinit();
        gui->board_.poweroff();
    }

    void UpdaterGui::restart_button_cb(lv_event_t *event) {
        void *arg = lv_event_get_user_data(event);
        auto *gui = static_cast<UpdaterGui*>(arg);

        esp_err_t ret = ESP_OK;

        const esp_partition_t *os_partition = esp_partition_find_first(
            ESP_PARTITION_TYPE_APP,
            ESP_PARTITION_SUBTYPE_APP_OTA_0,
            NULL
        );
        if (os_partition == NULL) {
            ESP_LOGE(TAG, "failed to get app ota0 partition");
            return;
        }

        ret = esp_ota_set_boot_partition(os_partition);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to set boot partition to os: %s", esp_err_to_name(ret));
            return;
        }

        gui->deinit();
        gui->board_.reboot();
    }

    void UpdaterGui::update_button_cb(lv_event_t *event) {
        xTaskCreate(
            update_task, "update_task",
            8 * 1024,
            lv_event_get_user_data(event),
            5, NULL
        );
    }

    void UpdaterGui::update_task(void *arg) {
        auto *gui = static_cast<UpdaterGui*>(arg);

        esp_err_t ret = ESP_OK;
        ret = gui->updater_.begin();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to update");
        }

        vTaskDelete(NULL);
    }
}
