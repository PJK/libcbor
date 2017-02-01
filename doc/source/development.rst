Development
==========================

Development dependencies
---------------------------
- `CMocka <http://cmocka.org/>`_ (testing)
- `Python <https://www.python.org/>`_ and `pip <https://pypi.python.org/pypi/pip>`_ (Sphinx platform)
- `Doxygen <http://www.stack.nl/~dimitri/doxygen/>`_
- `Sphinx <http://sphinx-doc.org/>`_ (documentation)
- There are some `Ruby <https://www.ruby-lang.org/en/>`_ scripts in ``misc``
- `Valgrind <http://valgrind.org/>`_ (memory correctness & profiling)
- `GCOV/LCOV <http://ltp.sourceforge.net/coverage/lcov.php>`_ (test coverage)


Installing *sphinx*
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

  pip install sphinx
  pip install sphinx_rtd_theme
  pip install breathe
  pip install https://github.com/lepture/python-livereload/archive/master.zip
  pip install sphinx-autobuild

Further instructions on configuring advanced features can be found at `<http://read-the-docs.readthedocs.org/en/latest/install.html>`_.


Live preview of docs
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

  cd doc
  make livehtml


Set up git hooks
~~~~~~~~~~~~~~~~~

A git hook that automatically refereshes the `GH pages <https://pages.github.com/>`_ contents located in ``docs`` can be symlinked:

.. code-block:: bash

  ln -s misc/hooks/pre-commit .git/hooks


Testing and code coverage
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please refer to :doc:`tests`
