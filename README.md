# display-thing
A simple picture frame thing that sits on your desk.

## Project Hardware

* **Microcontroller:** [ESP32 Development Board](https://www.aliexpress.com/item/1005006456519790.html)
* **Display:** [Waveshare 7.5inch e-Paper HAT (800x480)](https://www.waveshare.com/7.5inch-e-paper-hat.htm)
* **Frame:** [IKEA RÖDALM (13x18 cm)](https://www.ikea.com/de/en/p/roedalm-frame-oak-effect-10566390/)

## Wiring Table

| ESP32 Pin | Function          | E-Paper Display Pin |
|:----------|:------------------|:--------------------|
| `GPIO 23` | Power Enable      | `VCC` / `3.3V`      |
| `GPIO 18` | Chip Select (CS)  | `CS`                |
| `GPIO 5`  | Clock (CLK)       | `CLK`               |
| `GPIO 4`  | Data In (DIN)     | `DIN`               |
| `GPIO 19` | Data/Command (DC) | `D/C`               |
| `GPIO 21` | Reset (RST)       | `RST`               |
| `GPIO 22` | Busy              | `BUSY`              |
| `GND`     | Ground            | `GND`               |
