Next
---------------------
- Fix bad encoding of NaN half-floats [[Fixes #53]](https://github.com/PJK/libcbor/issues/53) (discovered by @BSipos-RKF)
    - **Warning**: Previous versions encoded NaNs as `0xf9e700` instead of `0xf97e00`; if you rely on the broken behavior, this will be a breaking change

0.6.0 (2020-03-15)
---------------------
- Correctly set .so version [[Fixes #52]](https://github.com/PJK/libcbor/issues/52). 
    - **Warning**: All previous releases will be identified as 0.0 by the linker.
- Fix & prevent heap overflow error in example code [[#74]](https://github.com/PJK/libcbor/pull/74) [[#76]](https://github.com/PJK/libcbor/pull/76) (by @nevun)
- Correctly set OSX dynamic library version [[Fixes #75]](https://github.com/PJK/libcbor/issues/75)
- [Fix misplaced 0xFF bytes in maps possibly causing memory corruption](https://github.com/PJK/libcbor/pull/82)
- BREAKING: Fix handling & cleanup of failed memory allocation in constructor
  and builder helper functions [[Fixes #84]](https://github.com/PJK/libcbor/issues/84)
  - All cbor_new_* and cbor_build_* functions will now explicitly return NULL when memory allocation fails
  - It is up to the client to handle such cases
- Globally enforced code style [[Fixes #83]](https://github.com/PJK/libcbor/issues/83)
- Fix issue possible memory corruption bug on repeated 
  cbor_(byte)string_add_chunk calls with intermittently failing realloc calls
- Fix possibly misaligned reads and writes when endian.h is uses or when
  running on a big-endian machine [[Fixes #99](https://github.com/PJK/libcbor/issues/99), [#100](https://github.com/PJK/libcbor/issues/100)]
- [Improved CI setup with Travis-native arm64 support](https://github.com/PJK/libcbor/pull/116)
- [Docs migrated to Sphinx 2.4 and Python3](https://github.com/PJK/libcbor/pull/117)

0.5.0 (2017-02-06)
---------------------
- Remove cmocka from the subtree (always rely on system or user-provided version)
- Windows CI
- Only build tests if explicitly enabled (`-DWITH_TESTS=ON`)
- Fixed static header declarations (by cedric-d)
- Improved documentation (by Michael Richardson)
- Improved `examples/readfile.c`
- Reworked (re)allocation to handle huge inputs and overflows in size_t [[Fixes #16]](https://github.com/PJK/libcbor/issues/16)
- Improvements to C++ linkage (corrected `cbor_empty_callbacks`, fixed `restrict` pointers) (by Dennis Bijwaard)
- Fixed Linux installation directory depending on architecture [[Fixes #34]](https://github.com/PJK/libcbor/issues/34) (by jvymazal)
- Improved 32-bit support [[Fixes #35]](https://github.com/PJK/libcbor/issues/35)
- Fixed MSVC compatibility [[Fixes #31]](https://github.com/PJK/libcbor/issues/31)
- Fixed and improved half-float encoding [[Fixes #5](https://github.com/PJK/libcbor/issues/5), [#11](https://github.com/PJK/libcbor/issues/11)]

0.4.0 (2015-12-25)
---------------------
Breaks build & header compatibility due to:

- Improved build configuration and feature check macros
- Endianess configuration fixes (by Erwin Kroon and David Grigsby)
- pkg-config compatibility (by Vincent Bernat)
- enable use of versioned SONAME (by Vincent Bernat)
- better fuzzer (wasn't random until now, ooops)

0.3.1 (2015-05-21)
---------------------
- documentation and comments improvements, mostly for the API reference

0.3.0 (2015-05-21)
---------------------

- Fixes, polishing, niceties across the code base
- Updated examples
- `cbor_copy`
- `cbor_build_negint8`, 16, 32, 64, matching asserts
- `cbor_build_stringn`
- `cbor_build_tag`
- `cbor_build_float2`, ...

0.2.1 (2015-05-17)
---------------------
- C99 support

0.2.0 (2015-05-17)
---------------------

- `cbor_ctrl_bool` -> `cbor_ctrl_is_bool`
- Added `cbor_array_allocated` & map equivalent
- Overhauled endianess conversion - ARM now works as expected
- 'sort.c' example added
- Significantly improved and doxyfied documentation

0.1.0 (2015-05-06)
---------------------

The initial release, yay!
