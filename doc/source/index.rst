
libcbor
===================================

Documentation for version |release|, updated on |today|.

Overview
--------
*libcbor* is a C library for parsing and generating CBOR_, the general-purpose schema-less binary data format.


Main features
 - Robust, standard compliant C11 [#]_ implementation
 - Performant, yet quite general – suitable for embedding or binding to higher level languages
 - Proper handling of UTF-8
 - Stream & chunked processing – works great with event-driven network applications
 - Extensive documentation and test suite
 - No runtime dependencies, small footprint
 - Thread-safe and scalable

.. [#] The code itself is C99 compliant, C11 features are only used for static assertions and it is therefore possible to build *libcbor* on most C99 platforms.

Contents
----------
.. toctree::

   getting_started
   api
   tests
   rfc_conformance
   internal




Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. _CBOR: http://tools.ietf.org/html/rfc7049
