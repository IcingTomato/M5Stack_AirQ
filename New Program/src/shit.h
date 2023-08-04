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
 * @param _scd_addr default 0x62
 * @param delay     default 5s
 */
void scdStartMeasure(uint16_t _scd_addr, int delay)
{
    Wire.beginTransmission(_scd_addr);
    Wire.write(0x21);
    Wire.write(0xb1);
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
 * @param _scd_addr             default 0x62
 * @param _scd_cmd_read_delay   default 5s
 * @return 
 */
scdData scdReadData(uint16_t _scd_addr, uint16_t _scd_cmd_read_delay)
{
    scdData scd;
    Wire.beginTransmission(_scd_addr);
    Wire.write(0xec);
    Wire.write(0x05);
    Wire.endTransmission();

    uint8_t _data[12], _counter;
    int _scd_cmd_read_len = 12;
    Wire.requestFrom((uint16_t)_scd_addr, (uint8_t)_scd_cmd_read_len);
    _counter = 0;
    while (Wire.available()) 
    {
        _data[_counter++] = Wire.read();
    }
    
    scd._co2 = (float)((uint16_t)_data[0] << 8 | _data[1]);
    scd._temperature = -45 + 175 * (float)((uint16_t)_data[3] << 8 | _data[4]) / 65536;
    scd._humidity = 100 * (float)((uint16_t)_data[6] << 8 | _data[7]) / 65536;

    vTaskDelay(_scd_cmd_read_delay / portTICK_PERIOD_MS);

    return scd;
}

/**
 * @brief 
 * 
 *
 * @param _sen_addr default 0x69
 * @param delay     default 5s
 */
void senStartMeasure(uint16_t _sen_addr, int delay)
{
    Wire.beginTransmission(_sen_addr);
    Wire.write(0x00);
    Wire.write(0x21);
    Wire.endTransmission();
    vTaskDelay(delay / portTICK_PERIOD_MS);
}

/**
 * @brief 
 * 
 */
typedef struct {
    float _pm1_0;
    float _pm2_5;
    float _pm4_0;
    float _pm10;
    float _humidity;
    float _temperature;
    float _voc;
    float _nox;
} senData;

/**
 * @brief 
 * 
 *
 * @param _sen_addr             default 0x69
 * @param _sen_cmd_read_delay   default 5s
 * @return 
 */
senData senReadData(uint16_t _sen_addr, uint16_t _sen_cmd_read_delay)
{
    senData sen;

    Wire.beginTransmission(_sen_addr);
    Wire.write(0x02);
    Wire.write(0x02);
    Wire.endTransmission();

    vTaskDelay(100 / portTICK_PERIOD_MS);

    uint8_t _data[2], _counter;
    int _sen_cmd_read_len = 3;
    _counter = 0;
    Wire.requestFrom((uint16_t)_sen_addr, (uint8_t)_sen_cmd_read_len);
    while (Wire.available()) 
    {
        _data[_counter++] = Wire.read();
    }

    if (_data[1] == 0x01)
    {
        Wire.beginTransmission(_sen_addr);
        Wire.write(0xc4);
        Wire.write(0x03);
        Wire.endTransmission();

        uint8_t _data[24], _counter;
        int _sen_cmd_read_len = 24;
        Wire.requestFrom((uint16_t)_sen_addr, (uint8_t)_sen_cmd_read_len);
        _counter = 0;
        while (Wire.available()) 
        {
            printf("0x%02x", _data[_counter++] = Wire.read());
        }
        printf("\n");
        // sen._pm1_0 = (float)((uint16_t)_data[0] << 8 | _data[1]) / 10;
        // sen._pm2_5 = (float)((uint16_t)_data[3] << 8 | _data[4]) / 10;
        // sen._pm4_0 = (float)((uint16_t)_data[6] << 8 | _data[7]) / 10;
        // sen._pm10 = (float)((uint16_t)_data[9] << 8 | _data[10]) / 10;
        // sen._humidity = (float)((uint16_t)_data[12] << 8 | _data[13]) / 100;
        // sen._temperature = (float)((uint16_t)_data[15] << 8 | _data[16]) / 100;
        // sen._voc = (float)((uint16_t)_data[18] << 8 | _data[19]) / 100;
        // sen._nox = (float)((uint16_t)_data[21] << 8 | _data[22]) / 100;
    }
    // else
    // {
    //     sen._pm1_0 = 0;
    //     sen._pm2_5 = 0;
    //     sen._pm4_0 = 0;
    //     sen._pm10 = 0;
    //     sen._humidity = 0;
    //     sen._temperature = 0;
    //     sen._voc = 0;
    //     sen._nox = 0;
    // }
    
    // while (Wire.available()) 
    // {
    //     if (_data[1] = Wire.read() == 0x01)
    //     {
    //         Wire.beginTransmission(_sen_addr);
    //         Wire.write(0x03);
    //         Wire.write(0xc4);
    //         Wire.endTransmission();

    //         uint8_t _data[23], _counter;
    //         int _sen_cmd_read_len = 24;
    //         Wire.requestFrom((uint16_t)_sen_addr, (uint8_t)_sen_cmd_read_len);
    //         _counter = 0;
    //         while (Wire.available()) 
    //         {
    //             printf("%02x" ,_data[_counter++] = Wire.read());
    //         }

    //         sen._pm1_0 = (float)((uint16_t)_data[0] << 8 | _data[1]) / 10;
    //         sen._pm2_5 = (float)((uint16_t)_data[3] << 8 | _data[4]) / 10;
    //         sen._pm4_0 = (float)((uint16_t)_data[6] << 8 | _data[7]) / 10;
    //         sen._pm10 = (float)((uint16_t)_data[9] << 8 | _data[10]) / 10;
    //         sen._humidity = (float)((uint16_t)_data[12] << 8 | _data[13]) / 100;
    //         sen._temperature = (float)((uint16_t)_data[15] << 8 | _data[16]) / 200;
    //         sen._voc = (float)((uint16_t)_data[18] << 8 | _data[19]) / 10;
    //         sen._nox = (float)((uint16_t)_data[21] << 8 | _data[22]) / 10;

    //         break;
    //     }
    // }

    vTaskDelay(_sen_cmd_read_delay / portTICK_PERIOD_MS);

    return sen;
}

#endif