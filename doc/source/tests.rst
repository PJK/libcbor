Tests
=============

Unit tests
--------------

There is a comprehensive test suite employing CMocka_. You can run all of them using ``ctest`` in the build directory. Individual tests are themselves runnable. Please refer to `CTest <http://www.cmake.org/Wiki/CMake/Testing_With_CTest>`_ documentation for detailed information on how to specify particular subset of tests.

.. _CMocka: http://cmocka.org/

Testing for memory leaks
------------------------

Every release is extensively tested for memory correctness. You can run these tests by passing the ``-D ExperimentalMemCheck`` flag to ``ctest``. [#]_

.. [#] Project should be configured with ``-DCMAKE_BUILD_TYPE=Debug`` to obtain meaningful description of location of the leak.


Code coverage
-------------------

Every release is inspected using `GCOV/LCOV <http://ltp.sourceforge.net/coverage/lcov.php>`_. Platform-independent code should be fully covered by the test suite.


.. code-block:: bash

  lcov --capture --directory . --output-file coverage.info
  genhtml coverage.info --output-directory out

Fuzz testing
-----------------

To ensure the code is correct and performs well, there is an additional measure in place -- a testing server is fed several GB of data (both valid and invalid) over the network, decodes them, then encodes them back to CBOR and sends them back to the fuzz testing client.

TODO
