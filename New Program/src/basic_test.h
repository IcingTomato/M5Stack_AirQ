#include <Arduino.h>
#include <Wire.h>

#ifndef __c
// #warning "This is a C++ compiler, not C. NB!"

/**
 * @brief I2C Scanner
 * 
 *
 * @param _sda_pin 
 * @param _scl_pin 
 * @param freq 
 */
void i2cScanner(int _sda_pin, int _scl_pin, uint32_t freq)
{
    Wire.begin(_sda_pin, _scl_pin, freq);
    uint8_t address;
    printf("\n\n");
    printf("---------------------------------------------------\n");
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    for(int i = 0; i < 128; i += 16)
    {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++)
        {
            address = i + j;
            Wire.beginTransmission(address);
            esp_err_t error = Wire.endTransmission();
            if (error == ESP_OK) 
                if (address != 0) 
                    printf("%02x ", address);
                else
                    printf("-- ");             
            else if (error == ESP_ERR_TIMEOUT) 
                printf("UU ");
            else 
                printf("-- ");  
        } 
        printf("\n");
    }
    printf("---------------------------------------------------\n");
    printf("\n\n");
    // Wire.end();
}

#endif