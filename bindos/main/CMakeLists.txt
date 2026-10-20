# SPDX-License-Identifier: GPL-2.0
# Author: xunicatt
# Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

file(GLOB_RECURSE SRCS *.cpp)

idf_component_register(
    INCLUDE_DIRS "include"
    SRCS ${SRCS}
)
