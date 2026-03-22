# Core data processing

## CLI usage

Basic (global statistics):

```bash
./build/debug/pdt_cli --in examples/sample.csv
```

Filter by sensor and export JSON report:

```bash
./build/debug/pdt_cli \
  --in examples/sample.csv \
  --per-sensor \
  --from 2026-02-18T10:00:00 \
  --to   2026-02-18T12:00:00 \
  --out examples/report.json
```

#### Anomaly detection

The CLI supports three anomaly detection methods:

- `zscore` — standard score relative to mean and standard deviation
- `iqr` — interquartile range based detection
- `mad` — median absolute deviation based detection

The threshold value is provided with `--z` and the method can be selected with `--anomaly-method`.

Examples:

**Z-score**
```bash
./build/debug/pdt_cli \
  --in examples/sample.csv \
  --sensor S1 \
  --z 2.5 \
  --anomaly-method zscore \
  --top 5
```

**IQR**
```bash
./build/debug/pdt_cli \
  --in examples/sample.csv \
  --sensor S1 \
  --z 1.5 \
  --anomaly-method iqr \
  --top 5
```

**MAD**
```bash
./build/debug/pdt_cli \
  --in examples/sample.csv \
  --sensor S1 \
  --z 3.0 \
  --anomaly-method mad \
  --top 5
```

Notes:

- --z currently acts as the anomaly threshold parameter for all supported methods
- --anomaly-method defaults to zscore
- --top limits the number of reported anomalies

#### Skipped rows (invalid input)

Display CSV rows that failed to parse:

```bash
./build/debug/pdt_cli \
  --in examples/sample.csv \
  --skipped
```

Example output:

```
Skipped CSV rows: 3
line 5: invalid,data,row
line 8: 2026-02-18T10:00:00,S1,abc
line 12: missing,value
```

#### JSON Output (example)

Example JSON report produced with `--out report.json`:

```json
{
  "mode": "per_sensor",
  "import": {
    "parsed_ok": 37,
    "skipped": 9
  },
  "filter": {
    "from": "2026-02-18T08:00:00",
    "to": "2026-02-18T18:15:00"
  },
  "data": {
    "total": 37,
    "filtered": 37
  },
  "stats_by_sensor": {
    "S1": {
      "count": 15,
      "min": -50,
      "max": 100,
      "mean": 14.82,
      "stddev": 28.1058
    }
  },
  "anomalies": {
    "method": "zscore",
    "threshold": 1,
    "top_n": 5,
    "mode": "per_sensor",
    "per_sensor": {
      "S1": {
        "count": 1,
        "top": [
          {"timestamp":"2026-02-18T12:45:00","sensor":"S1","value":-50,"score":-2.30629}
        ]
      },
      "S2": { "count": 0, "top": [] }
    }
  }
}

```

#### Input format

CSV with header:

```
timestamp,sensor,value
2026-02-18T10:00:00,S1,1.0
```

Notes:

- Timestamp format: `YYYY-MM-DDTHH:MM:SS`
- Invalid lines are skipped and reported
- Time filtering is inclusive
- Use `--skipped` to print invalid CSV rows with line numbers
- Use `--method <zscore|iqr|mad>` to choose the anomaly detection strategy
