/**
 * @file shit.h
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

#ifndef __c
// #warning "This is a C++ compiler, not C. NB!"

/**
 * @brief 
 * 
 *
 * @param _hold_pin 
 */
void powerHold(int _hold_pin)
{
    pinMode(_hold_pin, OUTPUT);
    digitalWrite(_hold_pin, HIGH);
    printf("Power hold pin set to HIGH\n");
}

/**
 * @brief 
 * 
 *
 * @param _scd_addr 
 * @param _scd_ctrl 
 * @param _scd_cmd 
 * @param delay 
 */
void scdStartMeasure(uint16_t _scd_addr, uint16_t _scd_data, uint16_t _scd_cmd, int delay)
{
    Wire.beginTransmission(_scd_addr);
    Wire.write(_scd_data);
    Wire.write(_scd_cmd);
    Wire.endTransmission();
    vTaskDelay(delay / portTICK_PERIOD_MS);
}

/**
 * @brief 
 * 
 */
typedef struct {
    float _co2;
    float _temperature;
    float _humidity;
} scdData;

/**
 * @brief 
 * 
 *
 * @param _scd_addr 
 * @param _scd_data 
 * @param _scd_cmd 
 * @param _scd_cmd_read_len 
 * @param _scd_cmd_read_delay 
 * @return 
 */
scdData scdReadData(uint16_t _scd_addr, uint16_t _scd_data, uint16_t _scd_cmd, uint16_t _scd_cmd_read_len, uint16_t _scd_cmd_read_delay)
{
    scdData scd;
    Wire.beginTransmission(_scd_addr);
    Wire.write(_scd_data);
    Wire.write(_scd_cmd);
    Wire.endTransmission();

    uint8_t _data[12], _counter;
    Wire.requestFrom((uint16_t)_scd_addr, (uint8_t)_scd_cmd_read_len);
    _counter = 0;
    while (Wire.available()) 
    {
        _data[_counter++] = Wire.read();
    }
    
    float _co2, _temperature, _humidity;
    scd._co2 = (float)((uint16_t)_data[0] << 8 | _data[1]);
    scd._temperature = -45 + 175 * (float)((uint16_t)_data[3] << 8 | _data[4]) / 65536;
    scd._humidity = 100 * (float)((uint16_t)_data[6] << 8 | _data[7]) / 65536;

    vTaskDelay(_scd_cmd_read_delay / portTICK_PERIOD_MS);

    return scd;
}

#endif