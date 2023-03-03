#include <Arduino.h>
#include <INA.h>  // Zanshin INA Library
#include <Wire.h>
#include <Arduino_GFX_Library.h>
// #include <Button.h>

// The max shunt voltage of INA219 is +/- 320mv, with a 0.1 Ohm shunt resister, the max current is 0.32/0.1 = 3.2A.
// The Zanduino/INA library automatically set the gain bases on shunt resister and max current, the code can be found in `INA_Class::initDevice()`
const uint32_t SERIAL_SPEED    = 115200;  ///< Use fast serial speed
const uint32_t SHUNT_MICRO_OHM = 100000;  ///< Shunt resistance in Micro-Ohm, e.g. 100000 is 0.1 Ohm
const uint16_t MAXIMUM_AMPS    = 3;       ///< Max expected amps, clamped from 1A to a max of 1022A
uint8_t        devicesFound    = 0;       ///< Number of INAs found

INA_Class INA;
Arduino_GFX *gfx;
// Button    button(D6);  // GPIO12

void setup()
{
    Serial.begin(SERIAL_SPEED);

    // Init LCD
    Arduino_DataBus *bus = new Arduino_HWSPI(D3, D8);
    gfx = new Arduino_SSD1283A(bus, D6, 2);
    gfx->begin();
    gfx->fillScreen(BLACK);
    analogWrite(D0, 128);  // GPIO16

    // Use GPIO12 as input consider display does not need MISO,
    // only works after SPI & tft are both started.
    // pinMode(D6, INPUT);
    // button.begin();

    // Init INAxxx via I2C
    Wire.begin();
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
    INA.setBusConversion(8500);             // Maximum conversion time 8.244ms
    INA.setShuntConversion(8500);           // Maximum conversion time 8.244ms
    INA.setAveraging(128);                  // Average each reading n-times
    INA.setMode(INA_MODE_CONTINUOUS_BOTH);  // Bus/shunt measured continuously
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

        // There is a slight skew when there is no power or no load. Partially fix it.
        // By doing this, the negative current cannot be measured, consider INA219 is not a full bi-direction power measurement IC.
        if (busMilliVolts == 0 || shuntMicroVolts < 0)
        {
            shuntMicroVolts = busMicroAmps = busMicroWatts = 0;
        }

        currentMillis = millis();
        deltaMillis   = currentMillis - lastMillis;
        lastMillis    = currentMillis;
        // microWattMills += busMicroWatts * (currentMillis - lastMillis);
        microWattMills += int64_t(busMilliVolts) * int64_t(busMicroAmps) * int64_t(deltaMillis) / 1000;

        sprintf(sprintfBuffer, "%2d %3d %s %7.4fV %9.4fmV %9.4fmA %9.4fmW shunt %d\n", i + 1, INA.getDeviceAddress(i),
                INA.getDeviceName(i), busMilliVolts / 1000.0, shuntMicroVolts / 1000.0, busMicroAmps / 1000.0,
                busMicroWatts / 1000.0, shuntMicroVolts);
        Serial.print(sprintfBuffer);

        gfx->setCursor(0, 0);
        gfx->fillRect(0, 0, 80, 10, BLACK);
        gfx->setTextColor(RED);
        gfx->printf("0x%2X %s\n", INA.getDeviceAddress(i), INA.getDeviceName(i));

        gfx->setCursor(0, 16);
        gfx->fillRect(48, 16, 84, 10, BLACK);
        gfx->setTextColor(YELLOW);
        gfx->printf("VOL.    %9.3f V", busMilliVolts / 1000.0);

        gfx->setCursor(0, 32);
        gfx->fillRect(48, 32, 84, 10, BLACK);
        gfx->setTextColor(RED);
        gfx->printf("CURRENT %9.3f mA", busMicroAmps / 1000.0);
        
        gfx->setCursor(0, 48);
        gfx->fillRect(48, 48, 84, 10, BLACK);
        gfx->setTextColor(GREEN);
        gfx->printf("SHUNT   %9.3f mV", shuntMicroVolts / 1000.0);

        gfx->setCursor(0, 64);
        gfx->fillRect(48, 64, 84, 10, BLACK);
        gfx->setTextColor(CYAN);
        gfx->printf("POWER   %9.3f mW", busMicroWatts / 1000.0);

        // 1mWh = 1000uW x 3600 x 1000ms = 3.6 x 10^9uWh
        gfx->setCursor(0, 80);
        gfx->fillRect(48, 80, 84, 10, BLACK);
        gfx->setTextColor(MAGENTA);
        gfx->printf("TOTAL   %9.3f mWh", microWattMills / 3600000000.0);

        gfx->setCursor(0, 96);
        gfx->fillRect(48, 96, 84, 10, BLACK);
        gfx->setTextColor(ORANGE);
        if (busMicroAmps == 0)
        {
            gfx->print("R               - ohm\n");
        }
        else 
        {
            gfx->printf("R       %9.3f ohm\n", float(busMilliVolts) / busMicroAmps * 1000.0);
        }
 
        gfx->setCursor(0, 112);
        gfx->fillRect(30, 112, 102, 10, BLACK);
        gfx->setTextColor(WHITE);
        gfx->printf("TIME %4d m %5.2f s\n", (currentMillis - startMillis) / 60000, (currentMillis - startMillis) % 60000 / 1000.0);

    }  // for-next each INA device loop

    // if (button.toggled())
    // {
    // }

    delay(100);  // Wait 100ms before next reading
}
