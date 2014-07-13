Getting started
==========================

Building libcbor
------------------

Prerequisites:
 - C99 compiler
 - CMake_ 2.8 or newer (might be also called ``cmakesetup``, ``cmake-gui`` or ``ccmake`` depending on the installed version and system)
 - C build system CMake can target (make, MSVC toolchain, Apple Xcode, ...)
 .. _CMake: http://cmake.org/

.. code-block:: bash

  # Assuming you are in the directory where you want to build
  cmake path_to_libcbor_dir
  make cbor
  # libcbor.a is now in the src dir

Development dependencies
---------------------------
- `CMocka <http://cmocka.org/>`_
- `Jansson <http://www.digip.org/jansson/>`_
- Python and *pip*
- `Sphinx <http://sphinx-doc.org/>`_

Building *cmocka*
~~~~~~~~~~~~~~~~~

.. code-block:: bash

  # Starting from libcbor source directory
  git submodule update test/cmocka
  cd test
  mkdir cmocka_build && cd cmocka_build
  cmake ../cmocka
  make -j 4
  make install

Installing *sphinx*
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

  pip install sphinx
  pip install sphinx_rtd_theme
  pip install https://github.com/lepture/python-livereload/archive/master.zip
  pip install sphinx-autobuild
