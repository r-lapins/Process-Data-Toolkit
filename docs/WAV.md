# WAV signal processing

The project includes a signal processing module for basic spectral analysis implemented in modern C++.

### Pipeline

```md
Signal (WAV / synthetic) → Segment Selection → Optional Windowing → FFT backend (CPU / CUDA) → Single-Sided Spectrum → Peak Detection → Dominant Spectral Components
```

### FFT backends

The spectrum computation is implemented using a backend abstraction:

- `CpuFftBackend`
  - Uses internal DFT / FFT implementation
  - Automatically selects DFT or FFT

- `CudaFftBackend` (optional)
  - Uses NVIDIA cuFFT
  - Accelerates FFT on GPU
  - Reuses plan and buffers for steady-state performance

### WAV CLI

Run:

```bash
./build/debug/pdt_wav_cli --in input.wav
./build/debug/pdt_wav_cli --in examples/HDSDR_20230515_072359Z_15047kHz_AF.wav
```

Example with explicit options:

```bash
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

```bash
--in          <file.wav>
--window      <none|hann|hamming>
--algorithm   <auto|dft|fft|cufft>
--from        <index>
--window-size <count>
--list-sizes
--threshold   <0..1>
--mode        <threshold-only|local-maxima>
--top         <count>
--out         <file.csv>
--out-r       <file.txt>
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

```md
Input file    : examples/HDSDR_20230515_072359Z_15047kHz_AF.wav
Sample rate   : 48000 Hz
Channels      : 1
Samples       : 949760
From sample   : 4736
Window size   : 512
Window        : hamming
Algorithm     : fft
Total time    : 38.3 ms
Threshold     : 0.2
Peak mode     : threshold-only
Detected peaks: 89 | showing top 15
  1. f = 7546.88 Hz    |X| = 8.33    bin = 161
  2. f = 7500.00 Hz    |X| = 5.80    bin = 160
```

The text report can be exported separately to a .txt file.

Generated CSV format:

```md
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

```bash
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

## Benchmark (synthetic signal)

Run:

```bash
./build/release/fft_benchmark
```

The benchmark compares:
- DFT
- CPU FFT
- CUDA FFT (cuFFT)

Measures:

- First-call → init (plan + alloc)
- Steady-state → real performance

Example output:

```md
=== First-call latency ===
       N      DFT [ms]      CPU [ms]     CUDA [ms]     Speedup
--------------------------------------------------------------
    1024         46.36          1.06          0.63        1.67
    2048             -          2.24          0.74        3.05
    4096             -          4.76          1.00        4.78
    8192             -          9.79          1.55        6.32

=== Steady-state throughput ===
       N      CPU [ms]     CUDA [ms]     Speedup
------------------------------------------------
    8192          9.69          1.03        9.40
   16384         20.32          2.05        9.89
   32768         42.49          4.02       10.57
   65536         91.27          8.12       11.25
```

CUDA outperforms CPU for larger FFT sizes in steady-state.

This demonstrates the expected complexity difference:

```md
DFT  ~ O(N²)
FFT  ~ O(N log N)
```

### WAV pipeline benchmark

Run:

```bash
./build/debug-cuda-nosan/fft_benchmark_wav --in <file.wav>
```

It includes:

- windowing
- FFT backend (CPU / CUDA)
- peak detection
- full SpectrumEngine pipeline

Unlike the synthetic FFT benchmark, this reflects real-world performance.

Example output:

```bash
./build/debug-cuda-nosan/fft_benchmark_wav --in <file.wav> --sizes 1024,2048,2050
```

```md
Backend   N         First [ms]      Avg engine [ms]   All peaks 
--------------------------------------------------------------------------------
CPU       1024      1.207           1.099             1         
CUDA      1024      230.391         0.161             1         

CPU       2048      2.367           2.286             5         
CUDA      2048      0.956           0.317             5         

CPU       2050      192.443         193.472           5         
CUDA      2050      1.448           0.322             5         
```

Notes:

- CPU backend falls back to DFT for non power-of-two sizes (e.g. 2050 → very slow)
- CUDA (cuFFT) supports more sizes → remains fast
- First-call includes: 
    - cuFFT plan creation 
    - GPU memory allocation
- Avg engine [ms] reflects steady-state performance (important for SDR / streaming)

---

## Algorithms

### WAV signal processing methods:

#### - Discrete Fourier Transform (DFT)

```md
X[k] = Σ x[n] · e^(−j2πkn/N),  k = 0..N−1
```

Current implementation is `O(N²)` and serves as a reference implementation.

#### - Fast Fourier Transform (FFT)

The project implements a radix-2 Cooley–Tukey FFT algorithm.

The FFT recursively decomposes the DFT into even and odd indexed samples:

```md
X[k] = E[k] + W_N^k · O[k]
X[k + N/2] = E[k] - W_N^k · O[k]
```

where:

```md
W_N^k = e^(−j2πk/N)
```

The algorithm requires the input size to be a power of two and has time complexity `O(N log N)`

#### - Spectral peak detection

Two strategies:

**ThresholdOnly**

```md
X[i] >= threshold_ratio · max(X)
```

**LocalMaxima**

```md
X[i] > X[i-1] && X[i] > X[i+1]
```
