Getting started
==========================

Building libcbor
------------------

Prerequisites:
 - C11 compiler
 - CMake_ 2.8 or newer (might also be called ``cmakesetup``, ``cmake-gui`` or ``ccmake`` depending on the installed version and system)
 - C build system CMake can target (make, MSVC toolchain, Apple Xcode, ...)
 .. _CMake: http://cmake.org/


**Executing the build**:

.. code-block:: bash

  # Assuming you are in the directory where you want to build
  cmake -DCMAKE_BUILD_TYPE=Release path_to_libcbor_dir
  make

Both the shared (``libcbor.so``) and the static (``libcbor.a``) libraries should now be in the ``src`` subdirectory.

If you want to pass custom configuration, please refer to `<http://stellarium.org/wiki/index.php/Configuring_Build_Options>`_.

**Optional: running the tests**:

.. code-block:: bash

  ctest

Sample output:

:: 

    Test project /tmp/tmp.Wi5mbHfzXI
	Start 1: type_4_test
    1/4 Test #1: type_4_test ......................   Passed    0.00 sec
	Start 2: type_2_test
    2/4 Test #2: type_2_test ......................   Passed    0.00 sec
	Start 3: type_0_test
    3/4 Test #3: type_0_test ......................   Passed    0.00 sec
	Start 4: type_1_test
    4/4 Test #4: type_1_test ......................   Passed    0.00 sec

    100% tests passed, 0 tests failed out of 4

    Total Test time (real) =   0.01 sec
