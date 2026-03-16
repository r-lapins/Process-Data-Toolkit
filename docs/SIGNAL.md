# Signal processing

The project includes a signal processing module for basic spectral analysis implemented in modern C++.

### Pipeline

```
Signal (WAV / synthetic) → Segment Selection → Optional Windowing → DFT / FFT → Single-Sided Spectrum → Peak Detection → Dominant Spectral Components
```

### Spectrum CLI (WAV input)

Run:

```
./build/debug/spectrum_cli --in input.wav
./build/debug/spectrum_cli --in examples/HDSDR_20230515_072359Z_15047kHz_AF.wav
```

Example with explicit options:

```
./build/debug/spectrum_cli \
  --in input.wav \
  --window hann \
  --from 0 \
  --bins 1024 \
  --threshold 0.4 \
  --mode local-maxima \
  --top 10 \
  --algorithm auto \
  --out output.csv
```

Supported options:

```
--in <file.wav>
--window <none|hann|hamming>
--from <index>
--bins <count>
--threshold <0..1>
--mode <threshold-only|local-maxima>
--top <count>
--algorithm <auto|dft|fft>
--out <file.csv>
```

What the spectrum CLI does:

1. Reads a WAV file
2. Decodes PCM16 mono samples
3. Extracts a selected sample range
4. Optionally applies a window function
5. Computes a single-sided spectrum using DFT or FFT
6. Detects spectral peaks
7. Reports dominant spectral peaks

Example output:

```
Input file   : examples/sample.wav
Sample rate  : 48000 Hz
Channels     : 1
Samples      : 949760
From sample  : 4736
Bins         : 512
Window       : hamming
Algorithm    : fft
Threshold    : 0.2
Peak mode    : local-maxima
Top peaks    : 2

Dominant peaks
-------------------------------------
1. f = 7312.5 Hz    |X| = 6.69779    (bin 78)
2. f = 7125 Hz    |X| = 2.93414    (bin 76)
```

The computed spectrum can be exported to CSV for further analysis.

Generated CSV format:

```
frequency_hz,magnitude
0,0.0001
46.875,0.0032
93.75,0.0211
...
```

The file can be opened in:

- Excel / LibreOffice
- Python (NumPy / Pandas)
- MATLAB

### Spectrum demo (synthetic signal)

The project also includes a small synthetic-signal demo:

```
./build/debug/spectrum_synth_demo
```

The demo:

1. Generates a synthetic signal with multiple sinusoids
2. Computes the spectrum
3. Detects spectral peaks
4. Reports dominant spectral peaks

Example signal:

x(t) = sin(2π·50t) + 0.5·sin(2π·120t)

Expected dominant spectral peaks:

50 Hz
120 Hz

---

## Benchmark

The repository includes a simple benchmark comparing the runtime of the naive Discrete Fourier Transform and the radix-2 Fast Fourier Transform.

Run:

```
./build/release/fft_benchmark
```

Example output:

```
N,DFT(ms),FFT(ms)
64,0.08,0.01
128,0.3,0.01
256,1.16,0.03
512,4.57,0.07
1024,18.02,0.14
2048,71.7,0.32
```

This demonstrates the expected complexity difference:

```
DFT  ~ O(N²)
FFT  ~ O(N log N)
```
