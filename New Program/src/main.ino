/**
 * @file main.ino
 * @author IcingTomato (icing@m5stack.com)
 * @brief 
 * @version 0.1
 * @date 2023-08-04
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>
#include <Wire.h>
#include "shit.h"
#include "basic_test.h"

#include "esp_log.h"

static const char *TAG = "farting";

static int SDA_PIN = 1;
static int SCL_PIN = 0;
static int HOLD_PIN = 7;

static uint16_t SCD_ADDR = 0x62;
static int SCD_READ_BYTE = 12;

void setup(void)
{
    powerHold(HOLD_PIN);
    i2cScanner(SDA_PIN, SCL_PIN, 200000UL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    scdStartMeasure(SCD_ADDR, 0x21, 0xb1, 5000);
}

void loop(void)
{
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    scdData data = scdReadData(SCD_ADDR, 0xec, 0x05, SCD_READ_BYTE, 5000);
    printf("%.2f %.2f %.2f\n", data._co2, data._temperature, data._humidity);
}