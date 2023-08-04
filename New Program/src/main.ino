/**
 * @file main.ino
 * @author IcingTomato (icing@m5stack.com)
 * @brief The honour & halo of a embeded software engineer.
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

static int SCL_PIN          = 0;
static int SDA_PIN          = 1;
static int INK_SCK          = 3;
static int INK_DC           = 4;
static int HOLD_PIN         = 7;

static uint16_t SCD_ADDR    = 0x62;
static int SCD_DELAY        = 5000;

static uint16_t SEN_ADDR    = 0x69;
static int SEN_DELAY        = 5000;

void setup(void)
{
    powerHold(HOLD_PIN);
    i2cScanner(SDA_PIN, SCL_PIN, 200000UL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // scdStartMeasure(SCD_ADDR, SCD_DELAY);
    senStartMeasure(SEN_ADDR, SEN_DELAY);
}

void loop(void)
{
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // scdData scd = scdReadData(SCD_ADDR, SCD_DELAY);
    // printf("SCD40: %.2f %.2f %.2f\n", 
    //         scd._co2, 
    //         scd._temperature, 
    //         scd._humidity
    //         );
    senData sen = senReadData(SEN_ADDR, SEN_DELAY);
    // printf("SEN55: %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f\n", 
    //         sen._pm1_0, 
    //         sen._pm2_5, 
    //         sen._pm4_0,
    //         sen._pm10,
    //         sen._humidity,
    //         sen._temperature,
    //         sen._voc,
    //         sen._nox
    //         );
}