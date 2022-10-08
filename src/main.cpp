#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <WiFi.h>
#include "I2C_BM8563.h"
#include "airqStartup.h"

// The used commands use up to 48 bytes. On some Arduino's the default buffer
// space is not large enough
// #define MAXBUF_REQUIREMENT 48 //For SEN55
#define MAXBUF_REQUIREMENT 256 //For SEN55

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
// #include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>
#define ENABLE_GxEPD2_GFX 1
// #define GxEPD2_DISPLAY_CLASS GxEPD2_3C
// #define GxEPD2_DRIVER_CLASS GxEPD2_154_Z90c // GDEH0154Z90 200x200
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_154_M09 // GDEW0154M09 200x200, JD79653A

//EInk IO settings
int BUSY_Pin = 21; 
int RES_Pin = 9; 
int DC_Pin = 4; 
int CS_Pin = 8; 
int SCK_Pin = 3; 
int SDI_Pin = 20; 

#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define GxEPD2_7C_IS_GxEPD2_7C true
#define GxEPD2_1248_IS_GxEPD2_1248 true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x) 
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x)
#define IS_GxEPD2_1248(x) IS_GxEPD(GxEPD2_1248_IS_, x)

#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#elif IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
#elif IS_GxEPD2_7C(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))
#endif
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ CS_Pin, /*DC=*/ DC_Pin, /*RST=*/ RES_Pin, /*BUSY=*/ BUSY_Pin));

SensirionI2CSen5x sen5x;
SensirionI2CScd4x scd4x;

#define BM8563_I2C_SDA 1
#define BM8563_I2C_SCL 0

I2C_BM8563 rtc(I2C_BM8563_DEFAULT_ADDRESS, Wire);


const char* myssid      = "M5-R&D";
const char* mypassword  = "echo\"password\">/dev/null";

const char* ntpServer = "ntp.ntsc.ac.cn";

static bool State;
int buttonStat = 0;

int powerButton_Hold = 7;
int powerButton_Wake = 6;
int Button      = 9;

const int longPressTime = 2000;
unsigned long currentBootTime = 0;
unsigned long pressedTime  = 0;

TaskHandle_t buttonStat_t = NULL;

void startUp();
void Task_buttonStat(void *param);
void creatButtonStatTask();
void wireSetup();
void initWiFi();
void rtcSetup();
void getTime();
void sen55Setup();
void sen55Cap();
void scd40Setup();
void scd40Cap();
void einkSetup();
void powerSetup();
void powerBatteryOff();
void einkDisplaySCD40(String(temperature), String(humidity), String(co2));
void einkDisplaySCD40_2(String(temperature), String(humidity), String(co2));
void einkDisplaySEN55(String massConcentrationPm2p5, String massConcentrationPm10p0, String ambientHumidity, String ambientTemperature, String noxIndex);
void einkDisplaySEN55_2(String massConcentrationPm2p5, String massConcentrationPm10p0, String ambientHumidity, String ambientTemperature, String noxIndex);

void setup() 
{
    Serial.begin(115200);
    // pinMode(Button, INPUT_PULLUP);
    powerSetup();

    creatButtonStatTask();

    einkSetup();
    startUp();
    wireSetup();
    initWiFi();
    rtcSetup();
    
    scd40Setup();
    sen55Setup();
    // delay(10000);
}

void loop() 
{
    getTime();
    sen55Cap();
    delay(5000);
    scd40Cap();
    // delay(30000);
}

void powerSetup()
{
    pinMode(powerButton_Hold, OUTPUT);
    pinMode(powerButton_Wake, INPUT_PULLUP);
    digitalWrite(powerButton_Hold, HIGH);
    // delay(100);
    // if (digitalRead(powerButton_Wake) == LOW)
    // {
    //     Serial.println("Power is on");
    //     digitalWrite(powerButton_Hold, LOW);
    //     while (digitalRead(powerButton_Wake) == LOW)
    //     {
    //         delay(100);
    //     }
    // }
}

void powerBatteryOff()
{
    digitalWrite(powerButton_Hold, HIGH);
    delay(100);
    digitalWrite(powerButton_Hold, LOW);
}

void creatButtonStatTask() 
{
    xTaskCreate(
        Task_buttonStat,
        "Task for button status",
        8*1024,
        NULL,
        1,
        &buttonStat_t
    );
}

void Task_buttonStat(void *param) 
{
    while (1) 
    {
        State = digitalRead(Button);
        if (State == 0) 
        {
            buttonStat += 1;
            Serial.printf("Button Times: %d\n", buttonStat);
            State = digitalRead(Button);
            
            if (buttonStat > 2) 
            {
                rtc.clearIRQ();
                rtc.SetAlarmIRQ(5);
                delay(1000);
                // disable battery output, will wake up after 5 sec, Sleep current is 1~2μA
                // buttonStat = 0;
                powerBatteryOff();
                // if usb is not connected, following code will not be executed;
                // esp_deep_sleep(5000000);
                // esp_deep_sleep_start();
                // vTaskDelete(buttonStat_t);   
            }
            while (State == 0) 
            {
                State = digitalRead(Button);
                delay(100);
            }
        }
        delay(100);
    }
    vTaskDelete(NULL);
}

void initWiFi()
{
    WiFi.disconnect(1, 1);
    WiFi.mode(WIFI_STA);
    Serial.printf("Connecting to %s ", myssid);
    WiFi.begin(myssid, mypassword);
    while (WiFi.status() != WL_CONNECTED) 
    {
        Serial.print('.');
        delay(1000);
    }

    Serial.println("CONNECTED");
    Serial.println(WiFi.localIP());
}

void rtcSetup() 
{
    // Set ntp time to local
    configTime(8 * 3600, 0, ntpServer);
    rtc.clearIRQ();
    // Init RTC
    rtc.begin();

    // Get local time
    struct tm timeInfo;
    if (getLocalTime(&timeInfo)) {
        // Set RTC time
        I2C_BM8563_TimeTypeDef timeStruct;
        timeStruct.hours   = timeInfo.tm_hour;
        timeStruct.minutes = timeInfo.tm_min;
        timeStruct.seconds = timeInfo.tm_sec;
        rtc.setTime(&timeStruct);

        // Set RTC Date
        I2C_BM8563_DateTypeDef dateStruct;
        dateStruct.weekDay = timeInfo.tm_wday;
        dateStruct.month   = timeInfo.tm_mon + 1;
        dateStruct.date    = timeInfo.tm_mday;
        dateStruct.year    = timeInfo.tm_year + 1900;
        rtc.setDate(&dateStruct);
    }
    WiFi.disconnect(1, 1);
}

void getTime() 
{
    I2C_BM8563_DateTypeDef dateStruct;
    I2C_BM8563_TimeTypeDef timeStruct;

    // Get RTC
    rtc.getDate(&dateStruct);
    rtc.getTime(&timeStruct);

    // Print RTC
    Serial.printf("%04d/%02d/%02d %02d:%02d:%02d\n",
                dateStruct.year,
                dateStruct.month,
                dateStruct.date,
                timeStruct.hours,
                timeStruct.minutes,
                timeStruct.seconds
            );
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    display.fillScreen(GxEPD_WHITE);
    do
    {
        uint16_t x = 45;
        uint16_t y = 60;
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold24pt7b);
        display.printf("%02d:%02d\n",
                    timeStruct.hours,
                    timeStruct.minutes
                );
        display.setCursor(x + 15, y + 30);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold9pt7b);
        display.printf("%04d/%02d/%02d\n",
                    dateStruct.year,
                    dateStruct.month,
                    dateStruct.date
                );
        display.setCursor(x + 25, y + 70);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold9pt7b);
        display.println("HAVE A");
        display.setCursor(x, y + 100);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeSansBold12pt7b);
        display.println("NICE DAY");
    }
    while (display.nextPage());
    // Wait
    delay(100);
}

void sen55Setup() 
{
    uint16_t errorSEN5X;
    char errorSEN5XMessage[256];
    errorSEN5X = sen5x.deviceReset();
    if (errorSEN5X) {
        Serial.print("Error trying to execute deviceReset(): ");
        errorToString(errorSEN5X, errorSEN5XMessage, 256);
        Serial.println(errorSEN5XMessage);
    }
    // Print SEN55 module information if i2c buffers are large enough
    float tempOffset = 0.0;
    errorSEN5X = sen5x.setTemperatureOffsetSimple(tempOffset);
    if (errorSEN5X) {
        Serial.print("Error trying to execute setTemperatureOffsetSimple(): ");
        errorToString(errorSEN5X, errorSEN5XMessage, 256);
        Serial.println(errorSEN5XMessage);
    } else {
        Serial.print("Temperature Offset set to ");
        Serial.print(tempOffset);
        Serial.println(" deg. Celsius (SEN54/SEN55 only");
    }
    // Start Measurement
    errorSEN5X = sen5x.startMeasurement();
    if (errorSEN5X) {
        Serial.print("Error trying to execute startMeasurement(): ...");
        errorToString(errorSEN5X, errorSEN5XMessage, 256);
        Serial.println(errorSEN5XMessage);
    }
}

void scd40Setup() 
{
    uint16_t errorSCD40;
    char errorSCD40Message[256];
    // stop potentially previously started measurement
    errorSCD40 = scd4x.stopPeriodicMeasurement();
    if (errorSCD40) {
        Serial.print("errorSCD40 trying to execute stopPeriodicMeasurement(): ");
        errorToString(errorSCD40, errorSCD40Message, 256);
        Serial.println(errorSCD40Message);
    }
    // Start Measurement
    errorSCD40 = scd4x.startPeriodicMeasurement();
    if (errorSCD40) {
        Serial.print("errorSCD40 trying to execute startPeriodicMeasurement(): ");
        errorToString(errorSCD40, errorSCD40Message, 256);
        Serial.println(errorSCD40Message);
    }
    Serial.println("Waiting for first measurement... (5 sec)");
}

void scd40Cap() 
{
    uint16_t errorSCD40;
    char errorSCD40Message[256];
    delay(100);
    // Read Measurement
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool isDataReady = false;
    errorSCD40 = scd4x.getDataReadyFlag(isDataReady);
    if (errorSCD40) 
    {
        Serial.print("errorSCD40 trying to execute readMeasurement(): ");
        errorToString(errorSCD40, errorSCD40Message, 256);
        Serial.println(errorSCD40Message);
        return;
    }
    if (!isDataReady) 
    {
        return;
    }
    errorSCD40 = scd4x.readMeasurement(co2, temperature, humidity);
    if (errorSCD40) 
    {
        Serial.print("errorSCD40 trying to execute readMeasurement(): ");
        errorToString(errorSCD40, errorSCD40Message, 256);
        Serial.println(errorSCD40Message);
    } 
    else if (co2 == 0) 
    {
        Serial.println("Invalid sample detected, skipping.");
    } 
    else 
    {
        // Serial.print("Co2:");
        // Serial.print(co2);
        // Serial.print("\n");
        // Serial.print("Temperature:");
        // Serial.print(temperature);
        // Serial.print("\n");
        // Serial.print("Humidity:");
        // Serial.println(humidity);
        einkDisplaySCD40_2(String(temperature), String(humidity), String(co2));
        
    }
}

void sen55Cap() 
{
    uint16_t errorSEN5X;
    char errorSEN5XMessage[256];
    delay(1000);
    // Read Measurement
    float massConcentrationPm1p0;
    float massConcentrationPm2p5;
    float massConcentrationPm4p0;
    float massConcentrationPm10p0;
    float ambientHumidity;
    float ambientTemperature;
    float vocIndex;
    float noxIndex;
    errorSEN5X = sen5x.readMeasuredValues(
        massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
        massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
        noxIndex);
    if (errorSEN5X) {
        Serial.print("Error trying to execute readMeasuredValues(): ");
        errorToString(errorSEN5X, errorSEN5XMessage, 256);
        Serial.println(errorSEN5XMessage);
    } else {
        // Serial.print("MassConcentrationPm1p0:");
        // Serial.print(massConcentrationPm1p0);
        // Serial.print("\n");
        // Serial.print("MassConcentrationPm2p5:");
        // Serial.print(massConcentrationPm2p5);
        // Serial.print("\n");
        // Serial.print("MassConcentrationPm4p0:");
        // Serial.print(massConcentrationPm4p0);
        // Serial.print("\n");
        // Serial.print("MassConcentrationPm10p0:");
        // Serial.print(massConcentrationPm10p0);
        // Serial.print("\n");
        // Serial.print("AmbientHumidity:");
        // if (isnan(ambientHumidity)) {
        //     Serial.print("n/a");
        // } else {
        //     Serial.print(ambientHumidity);
        // }
        // Serial.print("\n");
        // Serial.print("AmbientTemperature:");
        // if (isnan(ambientTemperature)) {
        //     Serial.print("n/a");
        // } else {
        //     Serial.print(ambientTemperature);
        // }
        // Serial.print("\n");
        // Serial.print("VocIndex:");
        // if (isnan(vocIndex)) {
        //     Serial.print("n/a");
        // } else {
        //     Serial.print(vocIndex);
        // }
        // Serial.print("\n");
        // Serial.print("NoxIndex:");
        // if (isnan(noxIndex)) {
        //     Serial.println("n/a");
        // } else {
        //     Serial.println(noxIndex);
        // }

        einkDisplaySEN55_2(String(massConcentrationPm2p5),  String(massConcentrationPm10p0), String(ambientHumidity), String(ambientTemperature), String(noxIndex));
        // Serial.println("Skipping SEN55 display");
    }
}

void wireSetup() 
{
    Wire.begin(1, 0);

    sen5x.begin(Wire);
    scd4x.begin(Wire);
}

void einkSetup()
{
    pinMode(BUSY_Pin, INPUT); 
    pinMode(RES_Pin, OUTPUT);  
    pinMode(DC_Pin, OUTPUT);    
    pinMode(CS_Pin, OUTPUT);    
    pinMode(SCK_Pin, OUTPUT);    
    pinMode(SDI_Pin, OUTPUT);
    Serial.println();
    Serial.println("Eink Setup.");
    display.init(115200);
    SPI.end();
    SPI.begin(SCK_Pin, -1, SDI_Pin, CS_Pin);
}

void startUp() 
{
    display.setRotation(0);
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        // display.drawInvertedBitmap(0, 0, gImage_airqStartup, 200, 200, GxEPD_RED); 
        display.drawInvertedBitmap(0, 0, gImage_startUp, 200, 200, GxEPD_BLACK);
        uint16_t x = 55;
        uint16_t y = 120; 
        display.setFont(&FreeSansBold9pt7b);
        display.setCursor(x, y + 60);
        display.setTextColor(GxEPD_BLACK);
        display.printf("Initializing...");
        delay(2000);
        // display.fillScreen(GxEPD_WHITE);
    }
    while (display.nextPage());
}

void einkDisplaySCD40(String(temperature), String(humidity), String(co2)) 
{
    display.setRotation(0);
    display.setFont(&FreeSansBold9pt7b);
    uint16_t x = 40;
    uint16_t y = 40;
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y - 20);
        display.setTextColor(GxEPD_BLACK);
        display.println("SCD40 Data");
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(x, y + 40);
        display.setTextColor(GxEPD_BLACK);
        display.printf("CO2:");
        display.setCursor(x + 80, y + 40);
        display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        display.println(co2);
    }
    while (display.nextPage());
    //Serial.println("helloWorld done");
}

void einkDisplaySEN55(String massConcentrationPm2p5, String massConcentrationPm10p0, String ambientHumidity, String ambientTemperature, String noxIndex) 
{
    display.setRotation(0);
    display.setFont(&FreeSansBold9pt7b);
    uint16_t x = 40;
    uint16_t y = 120;
    display.setFullWindow();
    // display.firstPage();
    do
    {
        // display.fillScreen(GxEPD_WHITE);
        display.setCursor(x, y - 20);
        display.setTextColor(GxEPD_BLACK);
        display.println("SEN55 Data");
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.printf("Temp: ");
        display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        display.setCursor(x + 80, y);
        display.println(ambientTemperature);
        display.setTextColor(GxEPD_BLACK);
        display.setCursor(x, y + 20);
        display.printf("Humi: ");
        display.setCursor(x + 80, y + 20);
        display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        display.println(ambientHumidity);
        display.setCursor(x, y + 40);
        display.setTextColor(GxEPD_BLACK);
        display.printf("PM2.5:");
        display.setCursor(x + 80, y + 40);
        display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        display.println(massConcentrationPm2p5);
        display.setCursor(x, y + 60);
        display.setTextColor(GxEPD_BLACK);
        display.printf("PM10:");
        display.setCursor(x + 80, y + 60);
        display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        display.println(massConcentrationPm10p0);
    }
    while (display.nextPage());
}

void einkDisplaySEN55_2(String massConcentrationPm2p5, String massConcentrationPm10p0, String ambientHumidity, String ambientTemperature, String noxIndex)
{
    display.setRotation(0);
    display.setFullWindow();
    // display.firstPage();
    do
    {
        // display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 100, gImage_blank, 200, 100, GxEPD_WHITE);
        // display.drawInvertedBitmap(0, 0, gImage_all_in_one, 200, 200, GxEPD_RED);
        display.drawInvertedBitmap(40, 120, gImage_tempPic, 50, 50, GxEPD_BLACK);
        uint16_t x = 100;
        uint16_t y = 155;
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.print(ambientTemperature.toInt());
        display.setFont(&FreeSansBold9pt7b);
        display.print("*C");
        // display.setCursor(x, y + 60);
        // display.setTextColor(GxEPD_BLACK);
        // display.setFont(&FreeSansBold18pt7b);
        // display.print(ambientHumidity.toInt());
        // display.setFont(&FreeSansBold9pt7b);
        // display.print("%");
        delay(2000);
    } while (display.nextPage());
    delay(2000);
    do
    {
        // display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(0, 100, gImage_blank, 200, 100, GxEPD_WHITE);
        // display.drawInvertedBitmap(0, 0, gImage_all_in_one, 200, 200, GxEPD_RED);
        display.drawInvertedBitmap(45, 120, gImage_humiPic, 50, 50, GxEPD_BLACK);
        uint16_t x = 110;
        uint16_t y = 155;
        display.setFont(&FreeSansBold18pt7b);
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.print(ambientHumidity.toInt());
        display.setFont(&FreeSansBold9pt7b);
        display.print("%");
        // display.setCursor(x, y + 60);
        // display.setTextColor(GxEPD_BLACK);
        // display.setFont(&FreeSansBold18pt7b);
        // display.print(ambientHumidity.toInt());
        // display.setFont(&FreeSansBold9pt7b);
        // display.print("%");
        delay(2000);
    } while (display.nextPage());
    
}

void einkDisplaySCD40_2(String(temperature), String(humidity), String(co2))
{
    display.setRotation(0);
    display.setFullWindow();
    // display.drawInvertedBitmap(0, 100, gImage_blank, 200, 100, GxEPD_WHITE);
    // display.firstPage();
    do
    {
        // display.fillScreen(GxEPD_WHITE);
        // display.drawInvertedBitmap(0, 0, gImage_all_in_one, 200, 200, GxEPD_RED);
        // display.drawInvertedBitmap(0, 100, gImage_blank, 200, 100, GxEPD_WHITE);
        display.drawBitmap(0, 100, gImage_blank, 200, 100, GxEPD_WHITE);
        // delay(1000);
        display.drawInvertedBitmap(35, 120, gImage_co2Pic, 50, 50, GxEPD_BLACK);
        uint16_t x = 110;
        uint16_t y = 155;
        display.setFont(&FreeSansBold18pt7b);
        // display.setCursor(x, y);
        // display.setTextColor(GxEPD_BLACK);
        // display.println(temperature + "'C");
        // display.setCursor(x, y + 60);
        // display.setTextColor(GxEPD_BLACK);
        // display.println(humidity + "%");
        display.setCursor(x, y);
        display.setTextColor(GxEPD_BLACK);
        display.println(co2);
        delay(2000);
    } while (display.nextPage());
}