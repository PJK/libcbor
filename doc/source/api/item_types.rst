Types of items
===============================================

Every :type:`cbor_item_t` has a :type:`cbor_type` associated with it - these constants correspond to the types specified by the `CBOR standard <https://www.rfc-editor.org/info/std94>`_:

.. doxygenenum:: cbor_type

To find out the type of an item, one can use

.. doxygenfunction:: cbor_typeof

Please note the distinction between functions like :func:`cbor_isa_uint()` and :func:`cbor_is_int()`. The following functions work solely with the major type value.


Binary queries
------------------------

Alternatively, there are functions to query each particular type.

.. warning:: Passing an invalid :type:`cbor_item_t` reference to any of these functions results in undefined behavior.

.. doxygenfunction:: cbor_isa_uint
.. doxygenfunction:: cbor_isa_negint
.. doxygenfunction:: cbor_isa_bytestring
.. doxygenfunction:: cbor_isa_string
.. doxygenfunction:: cbor_isa_array
.. doxygenfunction:: cbor_isa_map
.. doxygenfunction:: cbor_isa_tag
.. doxygenfunction:: cbor_isa_float_ctrl


Logical queries
------------------------

These functions provide information about the item type from a more high-level perspective

.. doxygenfunction:: cbor_is_int
.. doxygenfunction:: cbor_is_float
.. doxygenfunction:: cbor_is_bool
.. doxygenfunction:: cbor_is_null
.. doxygenfunction:: cbor_is_undef


Comparing items
------------------------

.. doxygenfunction:: cbor_structurally_equal

.. note::

   ``cbor_structurally_equal`` implements **structural** equality: every
   aspect of the encoding is significant, including integer width, float
   width, definite-vs-indefinite length, chunk boundaries, and map entry
   order.  This is intentionally stricter than the data-model equality
   defined by :rfc:`8949`.

   Key differences from data-model equality:

   * ``cbor_build_uint8(1)`` and ``cbor_build_uint16(1)`` are **not**
     structurally equal, even though both represent the integer ``1``.
   * Two maps with the same key-value pairs but in a different encoded
     order are **not** structurally equal. (:rfc:`8949` §5.6.1 defines
     maps as unordered sets of key-value pairs at the data-model level.)
   * Two indefinite-length byte strings that carry the same bytes split
     across different chunk boundaries are **not** structurally equal.
   * Floating-point NaN values with different bit patterns are **not**
     structurally equal.

   If you need data-model equality (e.g. ``uint8(1) == uint64(1)``),
   you must implement it on top of this library using the value
   accessors (``cbor_get_uint64``, etc.).

   **Complexity**: because structural equality short-circuits on the first
   mismatch and otherwise visits each byte of data exactly once, the
   runtime is *O(n)* in the encoded byte size of the items being compared.
   No additional memory is allocated.
