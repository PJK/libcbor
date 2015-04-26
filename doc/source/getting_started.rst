Getting started
==========================

Building libcbor
------------------

Prerequisites:
 - C11 compiler
 - `Ruby <https://www.ruby-lang.org/en/downloads/>`_
 - CMake_ 2.8 or newer (might also be called ``cmakesetup``, ``cmake-gui`` or ``ccmake`` depending on the installed version and system)
 - C build system CMake can target (make, MSVC toolchain, Apple Xcode, ...)
 .. _CMake: http://cmake.org/


**Executing the build**:

.. code-block:: bash

  # Assuming you are in the directory where you want to build
  cmake -DCMAKE_BUILD_TYPE=Release path_to_libcbor_dir
  make cbor

Both the shared (``libcbor.so``) and the static (``libcbor.a``) libraries should now be in the ``src`` subdirectory.

**Build configuration**


======================  ====================================================   ======================  =====================================================================================================================
Option                  Meaning                                                Default                 Possible values
----------------------  ----------------------------------------------------   ----------------------  ---------------------------------------------------------------------------------------------------------------------
CMAKE_C_COMPILER        C compiler to use                                      ``cc``                   ``gcc``, ``clang``, ``clang-3.5``, ...
CUSTOM_ALLOC            Allow custom ``malloc``?                               ``ON``                   ``ON``, ``OFF``
======================  ====================================================   ======================  =====================================================================================================================




If you want to pass other custom configuration options, please refer to `<http://stellarium.org/wiki/index.php/Configuring_Build_Options>`_.

**Optional: running the tests**:

.. code-block:: bash

  ctest

Sample output:

:: 

    Test project /tmp/tmp.epUydqtOhV
        Start 1: type_4_test
    1/6 Test #1: type_4_test ......................   Passed    0.00 sec
        Start 2: type_2_test
    2/6 Test #2: type_2_test ......................   Passed    0.03 sec
        Start 3: type_0_test
    3/6 Test #3: type_0_test ......................   Passed    0.02 sec
        Start 4: bitfield_test
    4/6 Test #4: bitfield_test ....................   Passed    0.03 sec
        Start 5: type_1_test
    5/6 Test #5: type_1_test ......................   Passed    0.02 sec
        Start 6: cbor_stream_decode_test
    6/6 Test #6: cbor_stream_decode_test ..........   Passed    0.02 sec

    100% tests passed, 0 tests failed out of 6

    Total Test time (real) =   0.11 sec

