Internal mechanics
==========================

Internal workings of libcbor are mostly derived from the specification. The purpose of this document is to describe technical choices made during design & implementation and to explicate the reasoning behind these choices.

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
