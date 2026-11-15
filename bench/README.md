# Benchmarks

A harness for catching performance regressions between two versions of libcbor
(see [#24](https://github.com/PJK/libcbor/issues/24)). It measures:

| Metric | How | Stability |
|---|---|---|
| `instructions` | Instructions executed, counted by callgrind | Exact for a given compiler, libc and CPU architecture |
| `allocs`, `reallocs` | Calls to the allocator, through `cbor_set_allocs` | Exact |
| `alloc_bytes`, `peak_bytes` | Bytes requested, and the peak of live bytes | Exact |
| `text_bytes` | Size of the library's code (`size`) | Exact |
| `ns_min`, `ns_median` | Wall-clock time per run | Noisy: use for local runs, not for gating |

The counts are the main metrics, following
[SQLite](https://www.sqlite.org/cpu.html) and rustc-perf. They are stable
enough to show changes well under 1%, also on shared CI runners.
Instruction counts don't model cache misses or branch mispredictions, so a
change that matters should also be checked with wall-clock time.

## Comparing two versions

```bash
bench/compare.sh --base master            # master vs. the working tree
bench/compare.sh --base v0.13.0 --head v0.14.0
bench/compare.sh --filter '^load/'        # only cbor_load cases
```

The script builds libcbor at both versions (shared, Release, no LTO by
default), builds this checkout's harness against each, and prints a Markdown
report with the changes that cross the thresholds in
[`compare.py`](compare.py) and the full results. `--static` and `--lto`
change the build; `--out DIR` keeps the builds and raw results; `--fail`
exits with status 1 on a regression.

Instruction counts need [valgrind](https://valgrind.org/). It is not
available on macOS on Apple Silicon. There, the script measures wall-clock
time instead, or run it in a Linux container:

```bash
docker run --rm -v "$PWD":/src -w /src ubuntu:24.04 bash -c \
  'apt-get update && apt-get install -y build-essential cmake valgrind python3 git &&
   git config --global --add safe.directory /src && bench/compare.sh'
```

Only compare results measured with the same compiler, libc, and
architecture. The report states its configuration.

## Running the harness directly

```bash
cmake -B build -DWITH_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build
build/bench/libcbor_bench list
build/bench/libcbor_bench time load/gld_document_x100
valgrind --tool=callgrind --toggle-collect='bench_measured*' \
  build/bench/libcbor_bench run load/gld_document_x100
```

`run` runs the case once to warm up, then once inside `bench_measured`. The
other commands are `alloc`, `time`, and `check`. `check` verifies that
`encode` and `serialize` reproduce every input byte for byte. Output is
tab-separated `case`, `metric`, `value`. Every case is `operation/input`.

## Operations

| Operation | Measures |
|---|---|
| `load` | `cbor_load`, then `cbor_decref` |
| `decode` | `cbor_stream_decode` with no-op callbacks |
| `serialize` | `cbor_serialize` of an item built beforehand |
| `build` | Building items with `cbor_build_*` and `cbor_new_*`, then `cbor_decref` |
| `encode` | The streaming encoder, `cbor_encode_*` |

`build` and `encode` replay a recording of the input's decoder callbacks, so
the decoder is not part of their cost.

## Inputs

- `gld_*`: payloads from
  [GLD.SerializerBenchmark](https://github.com/leo-gan/GLD.SerializerBenchmark),
  a multi-language serializer benchmark that includes libcbor
  ([#433](https://github.com/PJK/libcbor/issues/433)). `message` is a flat
  record of mixed types, `document` a nested record, `telemetry` mostly
  floats, `strings` an array of short strings, and `event` a small envelope.
  The `_x100` variants are arrays of 100 instances. They were generated at
  GLD commit `9c91799` by its C data generator (seed 42, default sizes) and
  encoded by its `libcbor-stream` row.
- Synthetic shapes, generated at startup: flat arrays and maps of integers,
  strings, and floats (`uints`, `negint_map`, `strings`, `floats`), a 64 KiB
  byte string, many tiny nested containers, 2000-deep nesting, and
  indefinite-length items. These cover the shapes measured in
  [#469](https://github.com/PJK/libcbor/pull/469).
- `hostile_*`: invalid inputs that declare far more items than they contain
  ([#418](https://github.com/PJK/libcbor/issues/418),
  [#431](https://github.com/PJK/libcbor/issues/431)). Only `load` runs them;
  their `peak_bytes` should stay small. Versions before 0.15.0 allocate
  gigabytes on them, so exclude them with `--filter` when comparing against
  older versions.

To add an input file, put it in `data/`, add it to `data_files` in
[`bench.c`](bench.c), and keep it small: every case runs under callgrind on
every comparison.
