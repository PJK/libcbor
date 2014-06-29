Internal mechanics
==========================

Internal workings of libcbor are mostly derived from the specification. The purpose of this document is to describe technical choices made during design & implementation and to explicate the reasoning behind these choices.

Terminology
---------------
=== =================  ===
MTB Major Type Byte    http://tools.ietf.org/html/rfc7049#section-2.1
=== =================  ===

Convetions
--------------
API symbols start with `cbor_` or `CBOR_` prefix, internal symbols have `_cbor_` or `_CBOR_` prefix.

Memory layout
---------------
CBOR is very dynamic in the sense that it contains many data elements of variable length, sometimes even indefinite length. This section describes internal representation of all CBOR data types.

Generally speaking, data items are stored in a contiguous block of memory, taking the following form [#]_
::

  cbor_item_t.data
  |
  V
  +------------------------+------------------------------------+-------------------------------------+
  |                        |                                    |                                     |
  |  METADATA_WIDTH bytes  |  <X>_METADATA_WIDTH bytes          |  fixed or variable number of bytes  |
  |  of generic metadata   |  of data-type specific metadata    |  specific to <X>                    |
  |                        |                                    |                                     |
  +------------------------+------------------------------------+-------------------------------------+
                                  (X is cbor_item_t.type)


.. [#] ``<X>_METADATA_WIDTH`` is used instead of union because it would be larger than necessary for some data types (TODO example)

Type 0
^^^^^^^^^^^^
Unsigned integers have a fixed size of either 1, 2, 4, or 8 bytes. ``UINT_METADATA_WITDTH`` is ``sizeof(cbor_uint_width)``, the enum describing these possibilities. Then the appropriate number of bytes representing the particular uint [#]_ follow.

.. [#] The integer is encoded in platform's native encoding using ``uint<X>_t``

Type 1
^^^^^^^^^^^^
Negative integers are very much the same as unsigned integers. Their memory layout is identical.

Unfortunately, the RFC specifies the smallest representable value to be :math:`-1 - (2^{64} - 1) = -2^{64}`, hence libcbor can provide no simple API for manipulation of these integers, since a suitable signed integral type might not exist (``int64_t`` is the widest signed type universally available, bounded at :math:`-2^{63}+1`)

TODO static assert check


Type 2
^^^^^^^^^^^^^^^


Coding style
-------------
This code loosely follows the `Linux kernel coding style <https://www.kernel.org/doc/Documentation/CodingStyle>`_. Tabs are tabs, and they are 4 characters wide.
