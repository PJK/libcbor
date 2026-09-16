Tests
=============

Unit tests
--------------

There is a comprehensive test suite employing CMocka_. You can run all of them using ``ctest`` in the build directory. Individual tests are themselves runnable. Please refer to `CTest <http://www.cmake.org/Wiki/CMake/Testing_With_CTest>`_ documentation for detailed information on how to specify particular subset of tests.

.. _CMocka: http://cmocka.org/

CBOR Working Group test vectors
-------------------------------

The `test vectors maintained by the IETF CBOR Working Group <https://github.com/cbor-wg/cbor-test-vectors>`_
are vendored in ``test/data/cbor-test-vectors`` and run by ``cbor_test_vectors_test``.
Every vector is decoded with :func:`cbor_load`, compared against the expected
value, and re-serialized; vectors that are not well-formed must be rejected. See
the ``README.md`` in the data directory for the container format and the pinned
upstream revision.

The few vectors that *libcbor* deliberately does not satisfy -- well-formed but
invalid inputs (see :ref:`decoder-validity`) and NaN payloads, which are
canonicalized on encoding -- are listed with their rationale in
``test/cbor_test_vectors_test.c``. The test fails if any of them starts passing,
so the list stays current.

Testing for memory leaks
------------------------

Every release is tested for memory correctness. You can run these tests by passing the ``-T memcheck`` flag to ``ctest``. [#]_

.. [#] Project should be configured with ``-DCMAKE_BUILD_TYPE=Debug`` to obtain meaningful description of location of the leak. You might also need ``--dsymutil=yes`` on OS X.


Code coverage
-------------------

Every release is inspected using `GCOV/LCOV <http://ltp.sourceforge.net/coverage/lcov.php>`_. Platform-independent code should be fully covered by the test suite. Simply run

.. code-block:: bash

  make coverage


or alternatively run ``lcov`` by hand using

.. code-block:: bash

  lcov --capture --directory . --output-file coverage.info
  genhtml coverage.info --output-directory out

Fuzz testing
-----------------

Every release is tested using a fuzz test. In this test, a huge buffer filled with random data is passed to the decoder. We require that it either succeeds or fail with a sensible error, without leaking any memory. This is intended to simulate real-world situations where data received from the network are CBOR-decoded before any further processing.
