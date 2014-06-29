Internal mechanics
==========================

Internal workings of *libcbor* are mostly derived from the specification. The purpose of this document is to describe technical choices made during design & implementation and to explicate the reasoning behind those choices.

Terminology
---------------
=== =================  ===
MTB Major Type Byte    http://tools.ietf.org/html/rfc7049#section-2.1
=== =================  ===

Conventions
--------------
API symbols start with ``cbor_`` or ``CBOR_`` prefix, internal symbols have ``_cbor_`` or ``_CBOR_`` prefix.


General notes on the API design
--------------------------------
The API design has two main driving priciples:

 1. Let the client manage the memory as much as possible
 2. Behave exactly as specified by the standard

Combining these two principles in practice turns out to be quite difficult. Indefinite-length strings, arrays, and maps require client to handle every fixed-size chunk explicitly in order to

 - ensure the client never runs out of memory due to *libcbor*
 - never ``realloc`` memory
 - allow proper handling of (streamed) data bigger than available memory

Unfortunately, due to memory management limitations and some RFC requirements, the author found himself having to choose between a convenient API that could *by design* never be standard compliant, and an inconvenient API. The approach of *libcbor* is the latter. It comes at an additional cost of requiring the client to have quite deep understanding of how CBOR works. Nonetheless, I believe firmly that it was the right choice to make, as further layers of abstractions may be added if needed. At least, they will have a solid foundation to build upon.

Memory layout
---------------
CBOR is very dynamic in the sense that it contains many data elements of variable length, sometimes even indefinite length. This section describes internal representation of all CBOR data types.

Generally speaking, data items are stored in a contiguous block of memory, taking the following form [#]_
::

  cbor_item_t.data
  |
  V
  +------------------------------+------------------------------------+-------------------------------------+
  |                              |                                    |                                     |
  |  _CBOR_METADATA_WIDTH bytes  |  <X>_METADATA_WIDTH bytes          |  fixed or variable number of bytes  |
  |  of generic metadata         |  of data-type specific metadata    |  specific to <X>                    |
  |                              |                                    |                                     |
  +------------------------------+------------------------------------+-------------------------------------+
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^--"The header"           "The data"--^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^--"The metadata"

         "The specific metadata"--^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

                                        (X is cbor_item_t.type)

.. [#] ``<X>_METADATA_WIDTH`` is used instead of union because it would be larger than necessary for some data types (TODO example)

Type 0
^^^^^^^^^^^^
Unsigned integers have a fixed size of either 1, 2, 4, or 8 bytes. ``UINT_METADATA_WITDTH`` is ``sizeof(cbor_uint_width)``, the enum describing these possibilities. Then the appropriate number of bytes representing the particular uint [#]_ follow.

.. [#] The integer is encoded in platform's native encoding using ``uint<X>_t``

Type 1
^^^^^^^^^^^^
Negative integers are very much the same as unsigned integers. Their memory layout is identical.

Unfortunately, the RFC specifies the smallest representable value to be :math:`-1 - (2^{64} - 1) = -2^{64}`, hence *libcbor* can provide no simple API for manipulation of these integers, since a suitable signed integral type might not exist (``int64_t`` is the widest signed type universally available, bounded at :math:`-2^{63}+1`)

TODO static assert check


Type 2
^^^^^^^^^^^^^^^
Bytestrings have either fixed or indefinite length. This is described by :type:`_cbor_bytesting_type_metadata` and reflected in the metadata.

::

  cbor_item_t.data
  |
  V
  +------------------------------+-----------------------------------------+--------------------------------------------+
  |                              |                                         |                                            |
  |  _CBOR_METADATA_WIDTH bytes  |  _CBOR_BYTESTRING_METADATA_WIDTH        |  (struct _cbor_bytesting_metadata).length  |
  |  of generic metadata         |  for a struct _cbor_bytesting_metadata  |  or sizeof(cbor_item_t *) bytes            |
  |                              |                                         |                                            |
  +------------------------------+-----------------------------------------+--------------------------------------------+
                                        (X is cbor_item_t.type)


The data is either a contiguous block of memory containing ``(struct _cbor_bytestring_metadata).length`` bytes, or a pointer to **the last** chunk read. It is initialized to ``NULL``. Upon reading an invalid chunk or failing to read a chunk, it will be set to ``NULL``. After calling :func:`cbor_bytestring_read_chunk`, the pointer is a valid, initialized :type:`cbor_item_t *`, or ``NULL``. Further manipulation (e.g. calling :func:`cbor_bytestring_get_chunk` and ``decref``-ing it) might break the guarantee.

Coding style
-------------
This code loosely follows the `Linux kernel coding style <https://www.kernel.org/doc/Documentation/CodingStyle>`_. Tabs are tabs, and they are 4 characters wide.
