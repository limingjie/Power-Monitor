#include <Arduino.h>
#include <Button.h>
#include <INA.h>  // Zanshin INA Library
#include <SPI.h>
#include <TFT_eSPI.h>  // Hardware-specific library
#include <Wire.h>

#include "iosevka15.h"

// https://github.com/Zanduino/INA/blob/master/examples/DisplayReadings/DisplayReadings.ino
const uint32_t SERIAL_SPEED    = 115200;  ///< Use fast serial speed
const uint32_t SHUNT_MICRO_OHM = 100000;  ///< Shunt resistance in Micro-Ohm, e.g. 100000 is 0.1 Ohm
const uint16_t MAXIMUM_AMPS    = 1;       ///< Max expected amps, clamped from 1A to a max of 1022A
uint8_t        devicesFound    = 0;       ///< Number of INAs found

INA_Class INA;
TFT_eSPI  tft = TFT_eSPI();
Button    button(D6);  // GPIO12

bool alert = false;

void ICACHE_RAM_ATTR AlertCallback()
{
    // INA226 Alert PIN is Open Drain, so low means alert.
    // Note: NodeMCU A0 is connected to a voltage divider, the ESP8266 ADC has a range of [0, 1.0] v.
    alert = analogRead(A0) < 512;
}

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(SERIAL_SPEED);

    // GPIO2
    attachInterrupt(digitalPinToInterrupt(D4), AlertCallback, CHANGE);

    Wire.begin();

    // TFT Backlight
    analogWrite(D0, 128);  // GPIO16

    // Begin SPI before begin TFT
    SPI.begin();

    tft.begin();
    tft.setRotation(1);  // portrait
    tft.fillScreen(TFT_BLACK);
    // https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Smooth%20Fonts/FLASH_Array/Font_Demo_2_Array/Font_Demo_2_Array.ino
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.loadFont(iosevka15);
    tft.setTextDatum(TL_DATUM);
    tft.setTextPadding(180);

    // Use GPIO12 as input consider display does not need MISO,
    // only works after SPI & tft are both started.
    // pinMode(D6, INPUT);
    button.begin();

    devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);  // Expected max Amp & shunt resistance
    while (devicesFound == 0)
    {
        Serial.println(F("No INA device found, retrying in 10 seconds..."));
        delay(10000);                                             // Wait 10 seconds before retrying
        devicesFound = INA.begin(MAXIMUM_AMPS, SHUNT_MICRO_OHM);  // Expected max Amp & shunt resistance
    }
    Serial.print(F(" - Detected "));
    Serial.print(devicesFound);
    Serial.println(F(" INA devices on the I2C bus"));
    INA.setBusConversion(1000);             // Maximum conversion time 8.244ms
    INA.setShuntConversion(1000);           // Maximum conversion time 8.244ms
    INA.setAveraging(32);                   // Average each reading n-times
    INA.setMode(INA_MODE_CONTINUOUS_BOTH);  // Bus/shunt measured continuously
    INA.alertOnBusOverVoltage(true, 3500);  // Trigger alert if over 3.5V on bus
}

void loop()
{
    /*!
     * @brief    Arduino method for the main program loop
     * @details  This is the main program for the Arduino IDE, it is an infinite loop and keeps on
     * repeating. In order to format the output use is made of the "sprintf()" function, but in the
     * Arduino implementation it has no support for floating point output, so the "dtostrf()" function
     * is used to convert the floating point numbers into formatted strings.
     * @return   void
     */
    static uint16_t      loopCounter = 0;     // Count the number of iterations
    static char          sprintfBuffer[100];  // Buffer to format output
    static uint16_t      busMilliVolts;
    static int32_t       shuntMicroVolts, busMicroAmps;
    static int64_t       busMicroWatts;
    static int64_t       microWattMills = 0;
    static unsigned long startMillis    = millis();
    static unsigned long lastMillis     = millis();
    static unsigned long currentMillis;
    static unsigned long deltaMillis;

    Serial.print(F("Nr Adr Type   Bus      Shunt       Bus         Bus\n"));
    Serial.print(F("== === ====== ======== =========== =========== ===========\n"));
    for (uint8_t i = 0; i < devicesFound; i++)  // Loop through all devices
    {
        busMilliVolts   = INA.getBusMilliVolts(i);
        shuntMicroVolts = INA.getShuntMicroVolts(i);
        busMicroAmps    = INA.getBusMicroAmps(i);
        busMicroWatts   = INA.getBusMicroWatts(i);
        // if (busMicroAmps < 0)
        // {
        //     busMicroAmps = 0;
        // }
        currentMillis = millis();
        deltaMillis   = currentMillis - lastMillis;
        lastMillis    = currentMillis;
        // microWattMills += busMicroWatts * (currentMillis - lastMillis);
        microWattMills += int64_t(busMilliVolts) * int64_t(busMicroAmps) * int64_t(deltaMillis) / 1000;

        sprintf(sprintfBuffer, "%2d %3d %s %7.4fV %9.4fmV %9.4fmA %9.4fmW shunt %d\n", i + 1, INA.getDeviceAddress(i),
                INA.getDeviceName(i), busMilliVolts / 1000.0, shuntMicroVolts / 1000.0, busMicroAmps / 1000.0,
                busMicroWatts / 1000.0, shuntMicroVolts);
        Serial.print(sprintfBuffer);

        sprintf(sprintfBuffer, "0x%2X %s\n", INA.getDeviceAddress(i), INA.getDeviceName(i));
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(sprintfBuffer, 0, 0);
        sprintf(sprintfBuffer, "%8.3fV %8.3fmA\n", busMilliVolts / 1000.0, busMicroAmps / 1000.0);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString(sprintfBuffer, 0, 16);
        // sprintf(sprintfBuffer, "Shunt = %9.5fmV\n", shuntMicroVolts / 1000.0);
        sprintf(sprintfBuffer, "%8.2fmW %8.2fmW\n", busMicroWatts / 1000.0,
                int64_t(busMilliVolts) * int64_t(busMicroAmps) / 1000000.0);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.drawString(sprintfBuffer, 0, 32);
        // 1mWh = 1000uW x 3600 x 1000ms = 3.6 x 10^9uWh
        sprintf(sprintfBuffer, "%8.3fmWh %3dm%04.1fs\n", microWattMills / 3600000000.0,
                (currentMillis - startMillis) / 60000, (currentMillis - startMillis) % 60000 / 1000.0);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString(sprintfBuffer, 0, 48);
        sprintf(sprintfBuffer, "%9.2fohm\n", float(busMilliVolts) / busMicroAmps * 1000.0);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString(sprintfBuffer, 0, 64);
    }  // for-next each INA device loop
    // Serial.println();
    // Serial.print(F("Loop iteration "));
    // Serial.print(++loopCounter);
    // Serial.print(F("\n\n"));

    // sprintf(sprintfBuffer, "Loop = %d\n", loopCounter);
    // tft.setTextColor(TFT_BLUE, TFT_BLACK);
    // tft.drawString(sprintfBuffer, 0, 64);

    if (button.toggled())
    {
        tft.fillRect(135, 30, 20, 20, button.read() == Button::PRESSED ? TFT_RED : TFT_BLACK);
    }

    tft.fillRect(135, 5, 20, 20, alert ? TFT_MAGENTA : TFT_BLACK);

    delay(100);  // Wait 1 seconds before next reading
}
