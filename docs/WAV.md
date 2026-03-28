# WAV signal processing

The project includes a signal processing module for basic spectral analysis implemented in modern C++.

### Pipeline

```
Signal (WAV / synthetic) → Segment Selection → Optional Windowing → DFT / FFT → Single-Sided Spectrum → Peak Detection → Dominant Spectral Components
```

### WAV CLI

Run:

```
./build/debug/pdt_wav_cli --in input.wav
./build/debug/pdt_wav_cli --in examples/HDSDR_20230515_072359Z_15047kHz_AF.wav
```

Example with explicit options:

```
./build/debug/pdt_wav_cli \
  --in input.wav \
  --window hann \
  --from 0 \
  --window-size 1024 \
  --threshold 0.4 \
  --mode local-maxima \
  --top 10 \
  --algorithm auto \
  --out output.csv \
  --out-r output.txt
  
```

Supported options:

```
--in <file.wav>
--window <none|hann|hamming>
--from <index>
--window-size <count>
--threshold <0..1>
--mode <threshold-only|local-maxima>
--top <count>
--algorithm <auto|dft|fft>
--out <file.csv>
--out-r <file.txt>
```

What the WAV CLI does:

1. Reads a WAV file
2. Decodes PCM16 mono samples
3. Extracts a selected sample range
4. Optionally applies a window function
5. Computes a single-sided spectrum using DFT or FFT
6. Detects spectral peaks
7. Selects dominant peaks sorted by magnitude
8. Prints a text report and optionally exports CSV / report files

Example text report:

```
Input file   : examples/HDSDR_20230515_072359Z_15047kHz_AF.wav
Sample rate  : 48000 Hz
Channels     : 1
Samples      : 949760
From sample  : 4736
Window size  : 512
Window       : hamming
Algorithm    : fft
Threshold    : 0.2
Peak mode    : threshold-only
Detected peaks: 89 | showing top 15
  1. f = 7546.88 Hz    |X| = 8.33    bin = 161
  2. f = 7500.00 Hz    |X| = 5.80    bin = 160
```

The text report can be exported separately to a .txt file.

Generated CSV format:

```
frequency_hz,magnitude
0,0.0001
46.875,0.0032
93.75,0.0211
...
```

The computed spectrum can be exported to CSV for further analysis.

Exported files can be opened in:

- Excel / LibreOffice for CSV
- Python (NumPy / Pandas) for CSV
- MATLAB for CSV
- Any text editor for the report

### Synthetic signal demo

The project also includes a small synthetic-signal demo:

```
./build/debug/pdt_wav_synth_demo
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
