Type 3 – UTF-8 strings 
=============================

CBOR strings have the same structure as :doc:`type_2_byte_strings`.

==================================  ======================================================
Corresponding :type:`cbor_type`     ``CBOR_TYPE_STRING``
Number of allocations (definite)    One plus any manipulations with the data
Number of allocations (indefinite)  One plus logarithmically many
                                    reallocations relative  to chunk count
Storage requirements (definite)     ``sizeof(cbor_item_t) + length(handle)``
Storage requirements (indefinite)   ``sizeof(cbor_item_t) * (1 + chunk_count) + chunks``
==================================  ======================================================


UTF-8 encoding validation
~~~~~~~~~~~~~~~~~~~~~~~~~~~
`RFC 8949 <https://www.rfc-editor.org/rfc/rfc8949#section-5.3.1>`_ treats
invalid UTF-8 in a text string as a *validity* error, not a *well-formedness*
error. In line with the policy described in :ref:`decoder-validity`,
*libcbor* decoders (:func:`cbor_load` and :func:`cbor_stream_decode`) do **not**
reject strings that are not valid UTF-8 — the item is built as-is and
the raw bytes are available through :func:`cbor_string_handle`.

Instead, the encoding is checked when a definite string is created or
its data is set, and the result is exposed through
:func:`cbor_string_codepoint_count`, which returns ``0`` for a string whose
data is not valid UTF-8. Note that an empty string also has ``0`` codepoints,
so a non-zero :func:`cbor_string_length` must be checked as well to tell the
two cases apart. Applications that require valid UTF-8 should check this after
decoding:

.. code-block:: c

    if (cbor_isa_string(item) && cbor_string_is_definite(item) &&
        cbor_string_length(item) > 0 && cbor_string_codepoint_count(item) == 0) {
      // Not valid UTF-8, handle according to your protocol's requirements
    }

For indefinite-length strings, check each chunk from
:func:`cbor_string_chunks_handle` individually. Note that per RFC 8949, every
chunk must be valid UTF-8 on its own; a multibyte codepoint must not be split
across chunks.

.. note::

    Prior to 0.11.0, invalid UTF-8 caused decoding to fail with
    ``CBOR_ERR_SYNTAXERROR``. This was
    `changed in 0.11.0 <https://github.com/PJK/libcbor/pull/286>`_ to
    follow the RFC 8949 distinction between well-formedness and validity.


Getting metadata
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_string_length
.. doxygenfunction:: cbor_string_codepoint_count
.. doxygenfunction:: cbor_string_is_definite
.. doxygenfunction:: cbor_string_is_indefinite
.. doxygenfunction:: cbor_string_chunk_count

Reading data
~~~~~~~~~~~~~

.. doxygenfunction:: cbor_string_handle
.. doxygenfunction:: cbor_string_chunks_handle

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_new_definite_string
.. doxygenfunction:: cbor_new_indefinite_string


Building items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. doxygenfunction:: cbor_build_string
.. doxygenfunction:: cbor_build_stringn


Manipulating existing items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_string_set_handle
.. doxygenfunction:: cbor_string_add_chunk
