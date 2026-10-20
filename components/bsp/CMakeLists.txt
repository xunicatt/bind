# SPDX-License-Identifier: GPL-2.0
# Author: xunicatt
# Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

file(GLOB_RECURSE SRCS *.cpp)

idf_component_register(
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    SRCS ${SRCS}
    REQUIRES esp_lcd_co5300 esp_lcd_touch_ft5x06
             lvgl esp_timer button esp_codec_dev
             qmi8658 pcf85063a fatfs littlefs ramfs
)
