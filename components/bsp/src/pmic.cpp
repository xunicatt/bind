// SPDX-License-Identifier: GPL-2.0
// Author: xunicatt
// Copyright (c) 2026 xunicatt <contact.aniket.biswas@gmail.com>

#include <bsp/pmic.h>
#include <esp_check.h>
#include <freertos/FreeRTOS.h>

extern "C" {
    #include <axp2101.h>
}

namespace bsp {
    static const char *TAG = "bsp::pmic";

    esp_err_t Pmic::init(I2c& i2c) {
        esp_err_t ret = ESP_OK;

        const i2c_device_config_t config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_ADDR,
            .scl_speed_hz = (100 * 1000),
            .scl_wait_us = 0,
            .flags = {
                .disable_ack_check = false,
            },
        };
        ret = axp2101_init_i2c(i2c.handle(), &config);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to initialize");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Disable CPUSLDO */
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_CPUSLDO, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable CPUSLDO-2");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* DCDC1 - 3.3V (V3V3) */
        ret = axp2101_set_dcdc_enabled(AXP2101_DCDC1, true);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to enable DCDC-1");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_dcdc_voltage(AXP2101_DCDC1, 3.3);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set DCDC-1 voltage");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Disable DCDC2 - DCDC5 */
        ret = axp2101_set_dcdc_enabled(AXP2101_DCDC2, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable DCDC-2");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_dcdc_enabled(AXP2101_DCDC3, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable DCDC-3");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_dcdc_enabled(AXP2101_DCDC4, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable DCDC-4");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_dcdc_enabled(AXP2101_DCDC5, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable DCDC-5");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* ALDO1 - 3.3V (A3V3) */
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO1, true);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to enable ALDO-1");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_ldo_voltage(AXP2101_LDO_ALDO1, 3.3);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set ALDO-1 voltage");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* ALDO2 - 3.3V (DISPLAY_EN) */
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO2, true);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to enable ALDO-2");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_ldo_voltage(AXP2101_LDO_ALDO2, 3.3);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set ALDO-2 voltage");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* ALDO3 - 1.5V (HAPTIC MOTOR) */
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO3, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable ALDO-3");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_ldo_voltage(AXP2101_LDO_ALDO3, 3.0);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set ALDO-3 voltage");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Disable ALDO4 */
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_ALDO4, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable ALDO-4");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Disable BLDO1 - BLDO2 */
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_BLDO1, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable BLDO-1");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_BLDO2, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable BLDO-2");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Disable DLDO1 - DLDO2 */
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_DLDO1, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable DLDO-1");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_ldo_enabled(AXP2101_LDO_DLDO2, false);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to disable DLDO-2");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* BATTERY - MAX CHRG VOLTAGE - 4.1V | MAX CHRG CURRENT - 125ma */
        ret = axp2101_set_charge_voltage(AXP2101_CHG_VOLT_4V20);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set charge voltage");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_charge_current(AXP2101_CHG_CURR_100MA);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set charge current");
        vTaskDelay(pdMS_TO_TICKS(10));

        ret = axp2101_set_vbus_measurement_enabled(true);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to enable vbus measurement");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_system_voltage_measurement_enabled(true);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to enable vsys measurement");
        vTaskDelay(pdMS_TO_TICKS(10));
        ret = axp2101_set_battery_voltage_measurement_enabled(true);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to enable vbat measurement");
        vTaskDelay(pdMS_TO_TICKS(10));

        uint8_t reg;

        ret = axp2101_get_irq_pwron_timing(&reg);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to get pwron timing");
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Longpress Time: 2 s */
        reg &= ~(0x3 << 0);
        reg |=  (0x3 << 0);
        reg &= ~(0x3 << 2);

        ret = axp2101_set_irq_pwron_timing(reg);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set pwron timing");
        vTaskDelay(pdMS_TO_TICKS(10));

        reg = 0;
        ret = axp2101_get_pwroff_en(&reg);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to get pwroff enable");
        vTaskDelay(pdMS_TO_TICKS(10));

        reg |= (1 << 1); /* Enable longpress */

        if (SHUTDOWN_ON_LONG_PRESS == 1) {
            reg &= ~(1 << 0); /* Shutdown (on longpress) when device is on */
        } else {
            reg |= (1 << 0); /* Restart (on longpress) when device is on */
        }

        ret = axp2101_set_pwroff_en(reg);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set pwroff enable");
        vTaskDelay(pdMS_TO_TICKS(10));

        return ESP_OK;
    }

    void Pmic::deinit() {
        axp2101_deinit();
    }

    Pmic::~Pmic() {
        deinit();
    }

    esp_err_t Pmic::poweroff() {
        esp_err_t ret = ESP_OK;
        uint8_t reg = 0;

        ret = axp2101_get_pmu_common_cfg(&reg);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to get pmu common config");
        vTaskDelay(pdMS_TO_TICKS(10));

        reg |= BIT(0);

        ret = axp2101_set_pmu_common_cfg(reg);
        ESP_RETURN_ON_ERROR(ret, TAG, "failed to set pmu common config");

        return ESP_OK;
    }
}
