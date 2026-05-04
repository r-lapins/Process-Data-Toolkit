# RTL-SDR live spectrum input

## Status
Experimental / optional module for live IQ streaming from RTL-SDR devices.

## Requirements
- librtlsdr
- pkg-config
- RTL-SDR USB dongle
- optional CUDA for faster FFT

## Build

```bash
cmake --preset debug-rtlsdr-cuda-nosan
cmake --build --preset debug-rtlsdr-cuda-nosan
```

## Run smoke test

```bash
./build/debug-rtlsdr-cuda-nosan/rtl_sdr_smoke
```

## Pipeline
RTL-SDR device → async IQ stream → uint8 IQ conversion → IqRingBuffer → IqSpectrumEngine → FFT backend → peak detection

## Configuration
- device index
- center frequency
- sample rate
- tuner gain

## Notes / limitations
- live/hardware-dependent path
- `rtl_sdr_smoke` currently uses hardcoded config
- CPU/CUDA IQ FFT currently expects power-of-two frame sizes
- tests cover IQ spectrum/ring buffer without requiring hardware
