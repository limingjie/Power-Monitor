# Power Monitor

Monitor the power consumption of ESP32 & ESP8266 devices and figure out how to save power. The DMM does not work well in this case as it is slow. And as an amateur hobbyist, a dedicated power monitor is not within the budget.

- First try using INA226
  ![Power Monitor INA226](Hardware/Power%20Monitor.png)
- Move to INA219
  ![INA219 on Breadboard](INA219%20on%20Breadboard.webp)
  ![INA219 with DMMs](INA219%20with%20DMMs.webp)
  ![Wiring](INA219%20BreadBoard%20Wiring.jpg)
  ![UI](INA219%20Power%20Monitor%20UI.jpg)

## Plan

- [x] Power monitoring IC selection
  - [x] ~~INA226~~
    - [x] ~~INA226 full address selection.~~
      - [x] ~~Use DPDT switches to implement 1-to-4 wire selection for both A1 and A0.~~
    - [x] ~~VBUS to VIN+ & VIN- selection.~~
    - [x] ~~Expose I2C interface.~~
    - [x] ~~Shunt resistor~~
      - [x] ~~Single shunt resistor.~~
      - [ ] ~~2 shunt resistors by a DPDT?~~
    - [x] ~~Reverse polarity protection.~~
      - ~~VBUS voltage - [0, 36] V.~~
  - [x] INA219
- [x] MCU selection - ESP8266
- [x] LCD selection
  - [x] 1.47" 135x240 TFT LCD
  - [x] 1.6" 132x132 SSD1283A
- [x] PCB design
  - [x] Business card size.
  - [x] Connectors
    - [ ] ~~Banana plug - Dropped since the size.~~
    - [x] KF301.
  - [x] Separate power supply by 18650.

## Progress

- [x] ~~Test INA226.~~
  - Got a CJMCU board with defect, the current measurement shows -16mA without any load, tried to fix it by code, it is not linear :(. The voltage measurement and the alert functions are working.
  - I also purchased ICs, but probably got defunct ones.
- [x] Test INA219.
  - [x] Works well on breadboard.
- [x] Test Si4909 reverse polarity protection.
- [x] PCB fabrication.
- [x] Ordering parts.
- [x] PCB soldering.
- [ ] [In progress] Coding.
  - [ ] Save data.
  - [ ] Sync data to web.

## Images

![Front 3D](Hardware/Power%20Monitor%20-%20Front%203D%20Rendering.png)
![Back 3D](Hardware/Power%20Monitor%20-%20Back%203D%20Rendering.png)
![Front](Hardware/Power%20Monitor%20-%20Front.png)
![Back](Hardware/Power%20Monitor%20-%20Back.png)
