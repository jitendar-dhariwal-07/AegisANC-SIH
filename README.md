# AegisANC

AI/ML-enabled adaptive noise cancellation and speech preservation proof of concept for SIH26052. The `frontend` is an offline-first simulator with honest hardware-disconnected states; `firmware` is a PlatformIO ESP32-S3 starter; `dashboard` is an optional Streamlit fallback.

## Run

```powershell
cd frontend
npm install
npm run dev
```

Choose Simulation mode to explore engine, generator, machinery, rotor, mixed, or quiet profiles. Hardware mode intentionally shows `Waiting for ESP32` until a serial integration is attached. Export CSV creates a local metric snapshot.

## Safety

Educational prototype only. It is not certified hearing protection. ANC is intended for continuous predictable noise, not explosions, firearms, or sudden battlefield noise. Unknown or impulsive signals should trigger SAFE MODE; keep a physical bypass and use low speaker volume.

See [WIRING.md](WIRING.md), [CALIBRATION.md](CALIBRATION.md), [PRESENTATION.md](PRESENTATION.md), and [DASHBOARD_SETUP.md](DASHBOARD_SETUP.md).
