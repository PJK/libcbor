Type 3 – UTF-8 strings 
=============================

CBOR strings work in much the same ways as :doc:`type_2`.

Streaming indefinite strings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please refer to :doc:`/streaming`.


Getting metadata
~~~~~~~~~~~~~~~~~

.. function:: size_t cbor_string_length(const cbor_item_t * item)

    Definite strings only

.. function:: bool cbor_string_is_definite(const cbor_item_t * item)
.. function:: bool cbor_string_is_indefinite(const cbor_item_t * item)

Reading data
~~~~~~~~~~~~~

.. function:: unsigned char * cbor_string_handle(const cbor_item_t * item)

	Returns a pointer to a contiguous area of memory containing :func:`cbor_string_length` bytes.


.. function:: cbor_item_t * * cbor_string_chunks_handle(const cbor_item_t * item)

    Indefinite strings only. Returns an array of chunks (definite strings)


.. function:: size_t cbor_string_chunk_count(const cbor_item_t * item)

    Indefinite strings only. Returns the number of chunks

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: cbor_item_t * cbor_new_definite_string()
.. function:: cbor_item_t * cbor_new_indefinite_string()

Building items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: cbor_item_t * cbor_string_add_chunk(cbor_item_t * item, cbor_item_t * chunk)

    Returns the original pointer on success, ``NULL`` on failure. Might :func:`realloc` the storage

.. function:: cbor_item_t * cbor_string_delete_chunk(cbor_item_t * item, size_t index)

Transformations
~~~~~~~~~~~~~~~~~~~

.. function:: cbor_item_t * cbor_string_concatenate(cbor_item_t * item)

    Create new definite length string by concatenation all chunks of an indefinite one.

