# Firmware setup

Install PlatformIO, open `firmware`, build for `esp32-s3-devkitc-1`, and upload over USB-C. The serial protocol is newline-delimited JSON at 115200 baud. Pin assignments and conservative startup values live in `include/config.h`. Production audio tasks should use fixed buffers, watchdog protection, gain ramping, limiter, and frozen adaptation on overload.

Before jury testing, verify the selected ESP32-S3 board's dual-microphone I2S routing. The included sketch is a measured single-mic test path because reference mic, error mic, and DAC routing cannot be assumed portable across ESP32-S3 Arduino cores. Validate with an oscilloscope or low-volume speaker and keep the physical bypass enabled.
