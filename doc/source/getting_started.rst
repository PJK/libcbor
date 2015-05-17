Getting started
==========================

Pre-built Linux packages are distributed from `the libcbor website <http://libcbor.org/>`_. For other platforms, you will need to compile it from source.

Building libcbor
------------------

Prerequisites:
 - C11 compiler
 - CMake_ 2.8 or newer (might also be called ``cmakesetup``, ``cmake-gui`` or ``ccmake`` depending on the installed version and system)
 - C build system CMake can target (make, Apple Xcode, MinGW, ...)
 .. _CMake: http://cmake.org/

.. note:: As of May 2015, the 2015 release candidate of Visual Studio does not support C11. While CMake will be happy to generate a VS solution that you can play with, libcbor currently cannot be compiled using the MSVC toolchain. Both `ICC <https://software.intel.com/en-us/c-compilers>`_ and GCC under `Cygwin <https://www.cygwin.com/>`_ will work.


**Configuration options**

A handful of configuration flags can be passed to `cmake`. The following table lists libcbor compile-time directives and several important generic flags.

======================  =======================================================   ======================  =====================================================================================================================
Option                  Meaning                                                   Default                 Possible values
----------------------  -------------------------------------------------------   ----------------------  ---------------------------------------------------------------------------------------------------------------------
``CMAKE_C_COMPILER``    C compiler to use                                         ``cc``                   ``gcc``, ``clang``, ``clang-3.5``, ...
``CUSTOM_ALLOC``        Allow custom ``malloc``?                                  ``ON``                   ``ON``, ``OFF``

``HUGE_FUZZ``           `Fuzz test </tests>` with 8GB of data                      ``OFF``                   ``ON``, ``OFF``
``SANE_MALLOC``         Assume ``malloc`` will refuse unreasonable allocations                   ``OFF``                   ``ON``, ``OFF``
``COVERAGE``            Generate test coverage instrumentation                    ``OFF``                   ``ON``, ``OFF``
``PRETTY_PRINTER``      Include a pretty-printer implementation                    ``ON``                   ``ON``, ``OFF``
``BUFFER_GROWTH``       Buffer growth factor                                       ``2``                     ``>1``
======================  =======================================================   ======================  =====================================================================================================================


If you want to pass other custom configuration options, please refer to `<http://www.cmake.org/Wiki/CMake_Useful_Variables>`_.

**Building using make**

.. code-block:: bash

  # Assuming you are in the directory where you want to build
  cmake -DCMAKE_BUILD_TYPE=Release path_to_libcbor_dir
  make cbor

Both the shared (``libcbor.so``) and the static (``libcbor.a``) libraries should now be in the ``src`` subdirectory.

In order to install the libcbor headers and libraries, the usual

.. code-block:: bash

  make install

is what your're looking for. Root permissions required on most systems.


**Building for ARM**

If you are building
