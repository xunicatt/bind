# SPDX-License-Identifier: GPL-2.0
# Author: xunicatt
# Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

idf_component_register(
    INCLUDE_DIRS "."
    SRCS "updater.cpp" 
    REQUIRES app_update board
)
