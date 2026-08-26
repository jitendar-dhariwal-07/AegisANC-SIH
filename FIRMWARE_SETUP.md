# Firmware setup

Install PlatformIO, open `firmware`, build for `esp32-s3-devkitc-1`, and upload over USB-C. The serial protocol is newline-delimited JSON at 115200 baud. Pin assignments and conservative startup values live in `include/config.h`. Production audio tasks should use fixed buffers, watchdog protection, gain ramping, limiter, and frozen adaptation on overload.

The firmware now captures the reference and error INMP441 microphones as LEFT/RIGHT stereo samples on I2S0 and drives the PCM5102A from I2S1. Verify the exact ESP32-S3 board's I2S pin routing and the microphone L/R selection before testing. Validate channel order with a low-volume speaker placed near the reference microphone only, then test with the physical bypass enabled.
