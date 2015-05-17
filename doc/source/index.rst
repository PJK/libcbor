
libcbor
===================================

Documentation for version |release|, updated on |today|.

Overview
--------
*libcbor* is a C library for parsing and generating CBOR_, the general-purpose schema-less binary data format.


Main features
 - Complete RFC conformance [#]_
 - Robust C11 implementation
 - Layered architecture offers both control and convenience
 - Flexible memory management
 - Proper handling of UTF-8
 - No shared global state - threading friendly
 - Full support for streams & incremental processing
 - Extensive documentation and test suite
 - No runtime dependencies, small footprint

.. [#] :doc:`rfc_conformance`

Contents
----------
.. toctree::

   getting_started
   using
   api
   streaming
   tests
   rfc_conformance
   internal
   changelog
   development

.. _CBOR: http://tools.ietf.org/html/rfc7049
