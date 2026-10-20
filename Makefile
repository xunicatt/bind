# SPDX-License-Identifier: GPL-2.0
# Author: xunicatt
# Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

PROJECT := bind
VERSION := 0.1

# Directories
DIR_REC := rebind
DIR_OS  := bindos
DIR_OUT := build

SOC       ?= esp32s3
BAUD_RATE ?= 921600

ifeq ($(IDF_PATH),)
  $(error "ERROR: esp-idf source not set")
endif

# Set Verbosity
VERBOSE ?= 0
Q := $(if $(filter 1,$(VERBOSE)),,@)
# Set Port
EXTRA_PORT := $(if $(PORT),--port $(PORT),)

# --- Base Commands ---
ESPTOOL := $(Q)esptool.py --chip $(SOC) -b $(BAUD_RATE) $(EXTRA_PORT) write_flash
IDF_REC := $(Q)idf.py -C $(DIR_REC) -B $(DIR_OUT)/$(DIR_REC)
IDF_OS  := $(Q)idf.py -C $(DIR_OS)  -B $(DIR_OUT)/$(DIR_OS)

# --- Offsets ---
GET_OFFSET = $(shell grep $(1) partitions.csv | cut -d ',' -f 4 | xargs)

O_BOOT := 0x0000000
O_PART := 0x0008000
O_OTA  := $(call GET_OFFSET,otadata)
O_REC  := $(call GET_OFFSET,rebind)
O_OS   := $(call GET_OFFSET,bindos)
O_FS   := $(call GET_OFFSET,storage)

# --- BINARIES ---
B_BOOT := $(DIR_OUT)/$(DIR_REC)/bootloader/bootloader.bin
B_PART := $(DIR_OUT)/$(DIR_REC)/partition_table/partition-table.bin
B_OTA  := $(DIR_OUT)/$(DIR_REC)/ota_data_initial.bin
B_REC  := $(DIR_OUT)/$(DIR_REC)/rebind.bin
B_OS   := $(DIR_OUT)/$(DIR_OS)/bindos.bin
B_FS   := $(DIR_OUT)/$(DIR_OS)/storage.bin

# --- GLOBAL TARGETS ---
.PHONY: all build flash clean fullclean monitor

all: build
build: build.rec build.os
clean: clean.rec clean.os

fullclean: fullclean.rec fullclean.os
	$(Q)rm -rf $(DIR_OUT)

monitor:
	$(Q)ESPPORT=$(PORT) idf.py \
			-C $(DIR_REC) \
			-B $(DIR_OUT)/$(DIR_REC) \
			monitor

# --- FLASH TARGETS ---
.PHONY: flash flash.boot flash.part flash.ota flash.fs

flash: build
	$(ESPTOOL) \
		$(O_BOOT) $(B_BOOT) \
		$(O_PART) $(B_PART) \
		$(O_OTA)  $(B_OTA)  \
		$(O_REC)  $(B_REC)  \
		$(O_OS)   $(B_OS)   \
		$(O_FS)   $(B_FS)

flash.boot: build.rec
	$(ESPTOOL) $(O_BOOT) $(B_BOOT)

flash.part: build.rec
	$(ESPTOOL) $(O_PART) $(B_PART)

flash.ota: build.rec
	$(ESPTOOL) $(O_OTA) $(B_OTA)

flash.fs: build.os
	$(ESPTOOL) $(O_FS) $(B_FS)

# --- RECOVERY (rebind) TARGETS ---
.PHONY: build.rec menuconfig.rec flash.rec clean.rec fullclean.rec

build.rec:
	$(IDF_REC) build

menuconfig.rec:
	$(IDF_REC) menuconfig

flash.rec: build.rec
	$(ESPTOOL) $(O_REC) $(B_REC)

clean.rec:
	$(IDF_REC) clean

fullclean.rec:
	$(IDF_REC) fullclean

# --- OS (bindos) TARGETS ---
.PHONY: build.os menuconfig.os flash.os clean.os fullclean.os

build.os:
	$(IDF_OS) app littlefs_storage_bin

menuconfig.os:
	$(IDF_OS) menuconfig

flash.os: build.os
	$(ESPTOOL) $(O_OS) $(B_OS)

clean.os:
	$(IDF_OS) clean

fullclean.os:
	$(IDF_OS) fullclean
