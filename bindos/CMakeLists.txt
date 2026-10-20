# SPDX-License-Identifier: GPL-2.0
# Author: xunicatt
# Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(bindos)

idf_component_get_property(lvgl_target lvgl__lvgl COMPONENT_LIB)
if(lvgl_target)
    target_compile_options(${lvgl_target} PRIVATE "-Wno-unused-function" "-Wno-attributes")
endif()

littlefs_create_partition_image(storage "${CMAKE_CURRENT_SOURCE_DIR}/../storage")
