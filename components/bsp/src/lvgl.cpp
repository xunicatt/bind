// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/bindings/lvgl.h>
#include <bsp/display.h>
#include <bsp/lvgl.h>
#include <bsp/sdcard.h>
#include <bsp/storage.h>
#include <dirent.h>
#include <esp_check.h>
#include <sys/param.h>
#include <unistd.h>

#ifndef CONFIG_FATFS_MAX_LFN 
#define CONFIG_FATFS_MAX_LFN 255
#endif

namespace bsp {
    static const char *TAG = "bsp::lvgl";
    static lv_display_t *s_display = nullptr;
    static char s_buffer[CONFIG_FATFS_MAX_LFN + 1];

    constexpr auto BUFFER_SIZE = (Display::WIDTH * lvgl::BUFFER_HEIGHT * sizeof(uint16_t));

    class LvglTask {
    public:
        static void tick_cb(void *arg) {
            ESP_UNUSED(arg);

            lv_tick_inc(TICK_PERIOD_MS);
        }

        static void task(void *arg) {
            uint32_t ms = 0;
            bool *run = static_cast<bool*>(arg);

            while (run != nullptr && *run) {
                ms = lv_timer_handler();
                ms = MAX(ms, TASK_MIN_DELAY_MS);
                ms = MIN(ms, TASK_MAX_DELAY_MS);

                usleep(ms * 1000);
            }

            vTaskDelete(nullptr);
        }

    private:
        static constexpr auto TICK_PERIOD_MS = 2;
        static constexpr auto TASK_MIN_DELAY_MS = (1000 / CONFIG_FREERTOS_HZ);
        static constexpr auto TASK_MAX_DELAY_MS = 500;

        friend Lvgl;
    };

    class LvglFs {
    public:
        static void register_drive(lv_fs_drv_t *drv, char letter,
                                   const char *mount) {
            lv_fs_drv_init(drv);

            drv->letter = letter;
            drv->user_data = (void*)mount;

            drv->open_cb = open_cb;
            drv->close_cb = close_cb;
            drv->read_cb = read_cb;
            drv->write_cb = write_cb;
            drv->seek_cb = seek_cb;
            drv->tell_cb = tell_cb;

            drv->dir_open_cb = dir_open_cb;
            drv->dir_close_cb = dir_close_cb;
            drv->dir_read_cb = dir_read_cb;

            lv_fs_drv_register(drv);
        }

        static void *open_cb(lv_fs_drv_t *drv, const char *path,
                          lv_fs_mode_t mode) {
            const char *base = static_cast<const char*>(drv->user_data);
            snprintf(s_buffer, sizeof(s_buffer), "%s/%s", base, path);

            const char *flags = "";
            if (mode == LV_FS_MODE_WR){
                flags = "w";
            } else if (mode == LV_FS_MODE_RD) {
                flags = "r";
            } else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) {
                flags = "r+";
            }

            return fopen(s_buffer, flags);
        }

        static lv_fs_res_t close_cb(lv_fs_drv_t *drv, void *file) {
            fclose(static_cast<FILE*>(file));
            return LV_FS_RES_OK;
        }

        static lv_fs_res_t read_cb(lv_fs_drv_t *drv, void *file,
                                void *buffer, uint32_t size, uint32_t *len) {
            lv_fs_res_t res = LV_FS_RES_OK;

            *len = fread(buffer, 1, size, static_cast<FILE*>(file));
            if (*len != size) {
                res = LV_FS_RES_UNKNOWN;
            }

            return res;
        }

        static lv_fs_res_t write_cb(lv_fs_drv_t *drv, void *file,
                                 const void *buffer, uint32_t size, uint32_t *len) {
            lv_fs_res_t res = LV_FS_RES_OK;

            *len = fwrite(buffer, 1, size, static_cast<FILE*>(file));
            if (*len != size) {
                res = LV_FS_RES_UNKNOWN;
            }

            return res;
        }

        static lv_fs_res_t seek_cb(lv_fs_drv_t *drv, void *file,
                                uint32_t pos, lv_fs_whence_t whence) {
            int seek = SEEK_SET;

            if (whence == LV_FS_SEEK_CUR) {
                seek = SEEK_CUR;
            } else if (whence == LV_FS_SEEK_END) {
                seek = SEEK_END;
            }

            fseek(static_cast<FILE*>(file), pos, seek);

            return LV_FS_RES_OK;
        }

        static lv_fs_res_t tell_cb(lv_fs_drv_t *drv, void *file, uint32_t *pos) {
            *pos = ftell(static_cast<FILE*>(file));
            return LV_FS_RES_OK;
        }

        static void *dir_open_cb(lv_fs_drv_t *drv, const char *path) {
            const char *base = static_cast<const char*>(drv->user_data);
            snprintf(s_buffer, sizeof(s_buffer), "%s/%s", base, path);
            return static_cast<void*>(opendir(s_buffer));
        }

        static lv_fs_res_t dir_read_cb(lv_fs_drv_t *drv, void *dir, char *buffer,
                                    uint32_t size) {
            struct dirent *entry = readdir(static_cast<DIR*>(dir));
            if (entry) {
                snprintf(
                    buffer, size, "%s%s",
                    entry->d_name,
                    (entry->d_type == DT_DIR) ? "/" : ""
                );
                return LV_FS_RES_OK;
            }

            buffer[0] = '\0';
            return LV_FS_RES_OK;
        }

        static lv_fs_res_t dir_close_cb(lv_fs_drv_t *drv, void *dir) {
            closedir(static_cast<DIR*>(dir));
            return LV_FS_RES_OK;
        }
    };

    esp_err_t Lvgl::init() {
        if (display_ != nullptr && indev_ != nullptr) {
            return ESP_OK;
        }

        BaseType_t res = pdFALSE;
        esp_err_t ret = ESP_OK;

        run_ = true;

        lv_init();

        buffer1_ = static_cast<uint8_t*>(heap_caps_aligned_alloc(
            64, BUFFER_SIZE,
            MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM
        ));
        ESP_GOTO_ON_FALSE(buffer1_ != nullptr, ESP_ERR_NO_MEM, err, TAG, "failed to allocate buffer 1");

        buffer2_ = static_cast<uint8_t*>(heap_caps_aligned_alloc(
            64, BUFFER_SIZE,
            MALLOC_CAP_DMA | MALLOC_CAP_SPIRAM
        ));
        ESP_GOTO_ON_FALSE(buffer2_ != nullptr, ESP_ERR_NO_MEM, err, TAG, "failed to allocate buffer 2");

        display_ = lv_display_create(Display::WIDTH, Display::HEIGHT);
        ESP_GOTO_ON_FALSE(display_ != nullptr, ESP_FAIL, err, TAG, "failed to register display");
        s_display = display_;

        lv_display_set_color_format(display_, LV_COLOR_FORMAT_RGB565_SWAPPED);
        lv_display_set_buffers(
            display_,
            buffer1_,
            buffer2_,
            BUFFER_SIZE,
            LV_DISPLAY_RENDER_MODE_PARTIAL
        );

        lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_STATE_DEFAULT);
        lv_display_set_flush_cb(display_, lvgl::flush_cb);
        lv_display_add_event_cb(
            display_, lvgl::invalidate_area_cb,
            LV_EVENT_INVALIDATE_AREA, nullptr
        );

        indev_ = lv_indev_create();
        ESP_GOTO_ON_FALSE(indev_ != nullptr, ESP_FAIL, err, TAG, "failed to register indev");

        lv_indev_set_type(indev_, LV_INDEV_TYPE_POINTER);
        lv_indev_set_read_cb(indev_, lvgl::touch_input_cb);
        lv_indev_set_display(indev_, display_);

        LvglFs::register_drive(&storage_, 'I', Storage::MOUNT_POINT);
        LvglFs::register_drive(&sdcard_,  'S', SDCard::MOUNT_POINT);

        args_.callback = LvglTask::tick_cb;
        args_.name = "lvgl_port_tick_inc_cb";

        ret = esp_timer_create(&args_, &timer_);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to create timer");
        ret = esp_timer_start_periodic(timer_, (LvglTask::TICK_PERIOD_MS * 1000));
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to start timer");

        res = xTaskCreate(
            LvglTask::task,
            "LVGL",
            TASK_STACK_SIZE,
            static_cast<void*>(&run_),
            TASK_PRIORITY,
            &task_
        );
        ESP_GOTO_ON_FALSE(res == pdTRUE, ESP_FAIL, err, TAG, "failed to create lvgl task");

        return ESP_OK;

    err:
        deinit();
        return ret;
    }

    void Lvgl::deinit() {
        run_ = false;

        vTaskDelay(pdMS_TO_TICKS(2 * LvglTask::TASK_MAX_DELAY_MS));

        if (timer_ != nullptr) {
            esp_timer_stop(timer_);
            esp_timer_delete(timer_);
            timer_ = nullptr;
        }

        if (indev_ != nullptr) {
            lv_indev_delete(indev_);
            indev_ = nullptr;
        }

        if (display_ != nullptr) {
            lv_display_delete(display_);
            display_ = nullptr;
            s_display = nullptr;
        }

        if (buffer2_ != nullptr) {
            heap_caps_free(buffer2_);
            buffer2_ = nullptr;
        }

        if (buffer1_ != nullptr) {
            heap_caps_free(buffer1_);
            buffer1_ = nullptr;
        }

        task_ = nullptr;
    }

    Lvgl::~Lvgl() {
        deinit();
    }

    esp_err_t Lvgl::suspend() {
        if (task_ == nullptr || timer_ == nullptr) {
            return ESP_ERR_INVALID_STATE;
        }

        esp_err_t ret = ESP_OK;

        vTaskSuspend(task_);
        ret = esp_timer_stop(timer_);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "failed to stop timer");

        return ESP_OK;

    err:
        vTaskResume(task_);
        return ret;
    }

    esp_err_t Lvgl::resume() {
        esp_err_t ret = ESP_OK;
        
        if (task_ == nullptr || timer_ == nullptr) {
            return ESP_ERR_INVALID_STATE;
        }

        ret = esp_timer_start_periodic(timer_, (LvglTask::TICK_PERIOD_MS * 1000));
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to start timer");
        vTaskResume(task_);

        lv_lock();
        {
            lv_obj_invalidate(lv_scr_act());
        }
        lv_unlock();

        return ESP_OK;
    }

    lv_display_t * Lvgl::display() {
        if (display_ != nullptr) {
            return display_;
        }

        return nullptr;
    }

    lv_indev_t * Lvgl::indev() {
        if (indev_ != nullptr) {
            return indev_;
        }

        return nullptr;
    }

    namespace lvgl {
        bool IRAM_ATTR flush_ready_cb(esp_lcd_panel_io_handle_t io,
                esp_lcd_panel_io_event_data_t *event, void *data) {
            if (s_display != nullptr) {
                lv_display_flush_ready(s_display);
            }

            return false;
        }
    }
}
