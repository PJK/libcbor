# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

libcbor is a C library for parsing and generating CBOR (RFC 8949), a schema-less binary data format. C99, MIT licensed, no runtime dependencies.

## Build Commands

```bash
# Configure (out-of-source build recommended)
cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_TESTS=ON -DSANITIZE=ON /path/to/libcbor

# Build
make

# Run all tests (requires -DWITH_TESTS=ON and CMocka installed)
ctest

# Run a single test
./test/array_test            # direct execution
ctest -R array_test          # via ctest

# Verbose test output
ctest -VV

# Format code (must pass before PR)
bash clang-format.sh

# Coverage (configure with -DCOVERAGE=ON)
make coverage                # GNU/LCOV
make llvm-coverage           # LLVM
```

Key CMake options: `BUILD_SHARED_LIBS`, `CBOR_PRETTY_PRINTER`, `CBOR_BUFFER_GROWTH`, `CBOR_MAX_STACK_SIZE`, `WITH_TESTS`, `WITH_EXAMPLES`, `COVERAGE`, `SANITIZE`.

## Architecture

The library is organized by CBOR type, each with a header, implementation, and test file:

- **`src/cbor.c`** — Main entry point: `cbor_load` (decoding), `cbor_serialize` (encoding), item builders
- **`src/cbor/*.c`** — Per-type modules: `arrays`, `bytestrings`, `strings`, `maps`, `ints`, `floats_ctrls`, `tags`
- **`src/cbor/serialization.c`** — Serialization of items to bytes
- **`src/cbor/streaming.c`** — Incremental/streaming decoder using callbacks
- **`src/cbor/encoding.c`** — Higher-level encoding interface
- **`src/cbor/internal/`** — Private utilities: `encoders` (low-level byte encoding), `loaders` (decoding), `builder_callbacks` (streaming→item construction), `stack` (decoding context), `memory_utils`, `unicode` (UTF-8 validation)

Core data structure is `cbor_item_t` (defined in `src/cbor/data.h`) with reference counting. Items are created with `cbor_build_*`/`cbor_new_*`, ownership managed via `cbor_incref`/`cbor_decref`, and `cbor_move()` provides move semantics.

## Code Conventions

- **Formatting**: Google-based clang-format, 2-space indent, 80-column limit. Run `bash clang-format.sh` before committing.
- **Naming**: Public API uses `cbor_*` prefix, internal functions use `_cbor_*` prefix.
- **Documentation**: Doxygen format for all public functions. Additional documentation referencing Doxygen symbols is in `doc/source` and is to be kept up to date.
- **Memory**: All allocations can fail; functions return NULL on allocation failure. Use `_CBOR_NOTNULL` guard macro for asserting allocation success internally.
- **Attributes**: `_CBOR_NODISCARD` on constructors/functions with important return values.
- **Tests**: CMocka framework. Each type module has a corresponding `*_test.c`. Test helpers in `test/assertions.c`, `test/test_allocator.c`, `test/stream_expectations.c`.
- **Header guards**: `#ifndef LIBCBOR_<NAME>_H` / `#define LIBCBOR_<NAME>_H` / `#endif // LIBCBOR_<NAME>_H`

## CI

CircleCI runs: formatting check (clang-format), cppcheck, GCC+Clang builds, 32-bit cross-compilation, ARM/MIPS/RISC-V builds, Windows MSVC, Valgrind memcheck, coverage. Cirrus CI tests FreeBSD. GitHub Actions runs OSS-Fuzz.

## Version management

The repository is <https://github.com/PJK/libcbor>. Use PR-driven workflow, every fix or feature should be on a separate branch that maps to a Github PR.

Changes relevant to the library clients are tracked in CHANGELOG.md. Each change should link the relevant PR and/or bug.

Merges use squash. Therefore, do not amend commits or force push them, simply add a new commit when adjusting a change.
