Next
-------

0.5.0 (2017-02-06)
---------------------
- Remove cmocka from the subtree (always rely on system or user-provided version)
- Windows CI
- Only build tests if explicitly enabled (`-DWITH_TESTS=ON`)
- Fixed static header declarations (by cedric-d)
- Improved documentation (by Michael Richardson)
- Improved `examples/readfile.c`
- Reworked (re)allocation to handle huge inputs and overflows in size_t [#16]
- Improvements to C++ linkage (corrected `cbor_empty_callbacks`, fixed `restrict` pointers) (by Dennis Bijwaard)
- Fixed Linux installation directory depending on architecture [#34] (by jvymazal)
- Improved 32-bit support [#35]
- Fixed MSVC compatibility [#31]
- Fixed and improved half-float encoding [#5] [#11]

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
