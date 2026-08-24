# Optional TinyML

Collect labelled 16 kHz windows for engine, generator, machinery, rotor, speech, alarm, quiet, and unknown. Extract RMS, peak, zero-crossing rate, dominant frequency, band energy, and optional MFCC/log-Mel features. A quantized int8 model can replace the baseline classifier when exported for TensorFlow Lite Micro; firmware must retain the baseline fallback.
