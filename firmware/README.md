# ESP32-S3 firmware

This firmware captures one INMP441 through I2S, runs a bounded NLMS test path, applies an output limiter, and reports measured RMS values as newline-delimited JSON at 115200 baud. Pin assignments are in `include/config.h`.

The current Arduino routing is intentionally a single-mic test path: the ESP32-S3 has two I2S controllers, but a complete reference mic, error mic, and PCM5102A design needs verified dual-mic routing on the exact board and Arduino core version. Do not present this firmware as a calibrated two-microphone ANC result until that routing is validated with the physical components.

This is a proof-of-concept signal path. Acoustic cancellation quality depends on microphone geometry, speaker/headphone response, secondary-path delay, gain calibration, and stable enclosure mechanics. Keep a physical bypass and test only at low volume.
