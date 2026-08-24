# Wiring

| Device | Signal | ESP32-S3 |
|---|---|---|
| INMP441 reference | BCLK / WS / DATA | 14 / 15 / 13 |
| INMP441 error | BCLK / WS / DATA | 14 / 15 / 16 |
| PCM5102A | BCLK / LRCLK / DIN | 4 / 5 / 6 |
| SSD1306 | SDA / SCL / address | 8 / 9 / 0x3C |
| Bypass button | signal | GPIO7, INPUT_PULLUP, active LOW |

Use 3.3 V logic, common ground, and never connect DAC output to an ESP32 GPIO. The firmware defaults are editable in `firmware/include/config.h`. If the board's I2S routing cannot expose two microphones, use a clearly labelled SINGLE_MIC_TEST_MODE and retain the physical bypass.
