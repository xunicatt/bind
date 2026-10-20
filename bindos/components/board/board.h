// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#ifndef BOARD_H
#define BOARD_H

#include <bsp/bsp.h>
#include <state.h>

namespace os {
    struct Board {
        esp_err_t init(State &state);
        void deinit();
        void poweroff();
        void reboot();

        bsp::RamFs ramfs;
        bsp::Storage storage;
        bsp::Buttons buttons;
        bsp::I2c i2c;
        bsp::Pmic pmic;
        bsp::Haptic haptic;
        bsp::SDCard sdcard;
        bsp::Display display;
        bsp::TouchInput touch;
        bsp::Lvgl lvgl;
        bsp::Rtc rtc;
        bsp::Imu imu;
        bsp::I2s i2s;
        bsp::Microphone microphone;
        bsp::Speaker speaker;
    };
}

#endif
