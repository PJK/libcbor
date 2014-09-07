Development
==========================

Development dependencies
---------------------------
- `CMocka <http://cmocka.org/>`_
- `Jansson <http://www.digip.org/jansson/>`_
- `Python <https://www.python.org/>`_ and `pip <https://pypi.python.org/pypi/pip>`_
- `Sphinx <http://sphinx-doc.org/>`_
- There are some `Ruby <https://www.ruby-lang.org/en/>`_ scripts in ``misc``
- `Valgrind <http://valgrind.org/>`_
- `GCOV/LCOV <http://ltp.sourceforge.net/coverage/lcov.php>`_

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

Further instructions on configuring advanced features can be found on `<http://read-the-docs.readthedocs.org/en/latest/install.html>`_.

Live preview of docs
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

  cd doc
  make livehtml

Testing and code coverage
~~~~~~~~~~~~~~~~~~~~~~

Please refer to :doc:`tests`