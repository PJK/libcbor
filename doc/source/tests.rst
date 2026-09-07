Tests
=============

Unit tests
--------------

There is a comprehensive test suite employing CMocka_. You can run all of them using ``ctest`` in the build directory. Individual tests are themselves runnable. Please refer to `CTest <http://www.cmake.org/Wiki/CMake/Testing_With_CTest>`_ documentation for detailed information on how to specify particular subset of tests.

.. _CMocka: http://cmocka.org/

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

Each input is also replayed with every allocation failing in turn, including
``realloc`` calls. These runs must return ``CBOR_ERR_MEMERROR`` and release all
allocations made before the failure. A fixed nested input exercises container
growth and partial cleanup even when the random inputs fail to parse early.
The test accepts an optional random seed (for example, ``test/fuzz_test 42``)
to reproduce a run.
