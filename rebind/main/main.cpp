// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <board.h>
#include <esp_check.h>
#include <updater.h>
#include <updater_gui.h>

static const char *TAG = "rebind::main";
static rebind::Board board = {};
static rebind::Updater updater(board);
static rebind::UpdaterGui gui(board, updater);

extern "C" int app_main(void) {
    esp_err_t ret = ESP_OK;

    ret = board.init();
    ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize: board");

    board.display.set_brightness(100);

    gui.init();

    if (board.sdcard.mounted()) {
        updater.init([](){
            gui.notify();
        });
    }

    return ESP_OK;
}
