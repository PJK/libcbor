# libcbor TODOs

## Memory Safety Bugs

### `claim_bytes()` unsigned subtraction
`src/cbor/streaming.c:13`

`provided - result->read` is computed as `size_t` (unsigned). If
`result->read > provided` due to a bug in a caller, this underflows to a huge
value, bypassing the bounds check. Rewriting as
`required + result->read > provided` would be safer.

## Encoding Correctness

### Implicit int shift in NaN encoding
`src/cbor/encoding.c:186`

`0x7FC0 << 16` is evaluated as `int` (not `uint32_t`). On systems with 32-bit
`int`, the result `0x7FC00000` overflows signed int range, which is undefined
behavior. Should be `(uint32_t)0x7FC0 << 16`.

### Half-float encoding edge cases
`src/cbor/encoding.c:154-170`

The `int8_t` cast of `exp - 127` can silently truncate when `exp = 255`
(produces 128, which overflows `int8_t`). There's an assertion for this, but in
release builds the assertion is gone and behavior is undefined.

### IEEE 754 assumption
`src/cbor/encoding.c`, `src/cbor/internal/loaders.c`

Multiple TODO comments acknowledge float encoding/decoding is broken on
non-IEEE 754 systems. Unlikely in practice but worth documenting as a hard
requirement.

## Build System

### Bump `cmake_minimum_required` to 3.9+
`CMakeLists.txt:1`

The minimum is currently 3.5, but the LTO code already has a
`VERSION_GREATER_EQUAL "3.9.0"` guard. Bumping to 3.9 would eliminate the guard
and the `CMP0069` policy workaround. Should be weighed against downstream
compatibility.

### Migrate remaining `CMAKE_C_FLAGS` sanitizer/coverage mutations
`CMakeLists.txt`

Sanitizer and coverage flags still use global `CMAKE_C_FLAGS` mutations because
they need both compile and link flags, and `target_link_options()` requires
CMake 3.13+. If the minimum is bumped to 3.13, these can move to
`target_compile_options()` + `target_link_options()` on the INTERFACE library.

## CI Gaps

### Sanitizers disabled in CI
`.circleci/config.yml:23,34`

Both build commands pass `-DSANITIZE=OFF`, so ASan/UBSan never run in CI.
Memory bugs are only caught by Valgrind (which is slower and catches fewer UB
issues).

### Windows CI doesn't build examples
The MSVC job doesn't pass `-DWITH_EXAMPLES=ON`, so Windows-specific build
issues in examples are never tested.

## Test Infrastructure

### Bug in test allocator mock
`test/test_allocator.c:19`

`calloc(calls, sizeof(expectations))` uses `sizeof(expectations)` which is
`sizeof(call_expectation*)` (pointer size), not `sizeof(call_expectation)`
(struct size). If the struct is larger than a pointer, the allocated buffer is
too small. This is the foundation of all allocation-failure tests.

### Thin UTF-8 test coverage
`test/unicode_test.c`

Only 2 test cases. No coverage for overlong encodings, surrogates, or boundary
code points.

## API Design Concerns

### Assertions instead of runtime checks in public API
All public API functions use `CBOR_ASSERT()` for type checking, which vanishes
in release builds. Wrong-type arguments cause silent UB rather than error
returns.

### `cbor_array_get()` on partially-constructed arrays
Can return NULL elements from partially-constructed definite arrays, which then
crash in `cbor_incref()`.

### Reference counting is not thread-safe
Simple `++`/`--` without atomics. The library doesn't claim thread-safety, but
it's undocumented.
