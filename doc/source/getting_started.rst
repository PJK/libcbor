Getting started
==========================

Building libcbor
------------------

Prerequisites:
 - C11 compiler
 - CMake_ 2.8 or newer (might be also called ``cmakesetup``, ``cmake-gui`` or ``ccmake`` depending on the installed version and system)
 - C build system CMake can target (make, MSVC toolchain, Apple Xcode, ...)
 .. _CMake: http://cmake.org/

.. code-block:: bash

  # Assuming you are in the directory where you want to build
  cmake path_to_libcbor_dir
  make cbor
  # libcbor.a is now in the src dir

