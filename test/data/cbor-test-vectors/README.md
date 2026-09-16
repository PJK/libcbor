# CBOR Working Group test vectors

Vendored copy of the `.cbor` containers from
<https://github.com/cbor-wg/cbor-test-vectors>, licensed under BSD-2-Clause
(see `LICENSE`).

Pinned commit: `7e84843b646676a715d4da21c719f263e4c43440` (2026-01-26)

Each `.cbor` file is a test suite: a map with `title`, `description`, an
optional file-level `fail` flag, and `tests`, an array of maps with

- `description`
- `encoded`: byte string with the CBOR to decode
- `decoded`: the expected value (optional)
- `roundtrip`: whether re-encoding the decoded value must yield `encoded`
  (default `true`)
- `fail`: whether decoding must fail (default: the file-level flag)

The `.edn` sources these were generated from are not vendored; the
containers are consumed directly by `test/cbor_test_vectors_test.c` using
libcbor itself.

To update, copy the `.cbor` files from `tests/` in the upstream repository
at the desired commit, update the pin above, run the test, and reconcile the
expected-failure list in the test source.
