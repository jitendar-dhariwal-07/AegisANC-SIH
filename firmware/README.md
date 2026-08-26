# ESP32-S3 firmware

This firmware captures one INMP441 through I2S, runs a bounded NLMS test path, applies an output limiter, and reports measured RMS values as newline-delimited JSON at 115200 baud. Pin assignments are in `include/config.h`.

The Arduino routing uses both INMP441 microphones as a stereo I2S input: reference is LEFT and error is RIGHT on the shared DATA line. This build has no DAC output: it performs two-microphone monitor-side adaptive estimation, reports measured RMS telemetry over USB, and updates the OLED. Add a DAC/headphone amplifier before claiming audible anti-noise output.

This is a proof-of-concept signal path. Acoustic cancellation quality depends on microphone geometry, speaker/headphone response, secondary-path delay, gain calibration, and stable enclosure mechanics. Keep a physical bypass and test only at low volume.
