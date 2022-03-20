# Power Monitor

Monitor the power consumption of ESP32 & ESP8266 devices and figure out how to save power. The DMM does not work well in this case as it is slow. And as a hobby, a dedicated power monitor is not within the budget.

## Plan

- [ ] Power monitor IC selection - INA226
  - [x] INA226 full address selection.
    - [x] Use DPDT to implement 1-to-4 wire selection for both A1 and A0.
  - [x] VBUS to VIN+ & VIN- selection.
  - [x] Expose I2C interface.
  - [ ] Shunt resistor
    - [x] Single shunt resistor.
    - [ ] 2 shunt resistors by a DPDT?
  - [x] Reverse polarity protection.
    - VBUS voltage - [0, 36] V.
- [x] MCU selection - ESP8266
- [x] Screen selection - 1.47" 135x240 TFT LCD
- [ ] PCB design
  - [x] Business card size.
  - [x] Connectors
    - [ ] ~~Banana plug - Dropped since the size.~~
    - [x] KF301.
  - [ ] Separate power supply by 18650.

## Progress

- [x] Test INA226.
  - Got a CJMCU board with defect, the current measurement shows -16mA without any load, tried to fix it by code, it is not linear :(. The voltage measurement and the alert functions are working.
- [x] Test Si4909 reverse polarity protection.
- [ ] [In progress] PCB fabrication.
- [x] Ordering parts.
- [ ] PCB soldering.
- [ ] [In progress] Coding.

## Images

![Front 3D](Hardware/Power%20Monitor%20-%20Front%203D%20Rendering.png)

![Front](Hardware/Power%20Monitor%20-%20Front.png)

![Back](Hardware/Power%20Monitor%20-%20Back.png)
