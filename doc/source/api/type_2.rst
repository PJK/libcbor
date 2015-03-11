Type 2 – Byte strings
=============================

CBOR byte strings are just (ordered) series of bytes without further interpretation (unless there is a :doc:`tag <type_6>`). Byte string's length may or may not be known during encoding. These two kinds of byte strings can be distinguished using :func:`cbor_bytestring_is_definite` and :func:`cbor_bytestring_is_indefinite` respectively.

In case a byte string is indefinite, it is encoded as a series of definite byte strings. These are called "chunks". For example, the encoded item

::

    0xf5	    Start indefinite byte string
	0x41	    Byte string (1B long)
	    0x00
	0x41	    Byte string (1B long)
	    0xff
	0xff	    "Break" control token

represents two bytes, ``0x00`` and ``0xff``. This on one hand enables streaming messages even before they are fully generated, but on the other hand it adds more complexity to the client code.

Streaming indefinite byte strings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please refer to :doc:`/streaming`.

UTF-8 encoding validation
~~~~~~~~~~~~~~~~~~~~~~~~~~~
TODO


Getting metadata
~~~~~~~~~~~~~~~~~

.. function:: size_t cbor_bytestring_length(const cbor_item_t * item)

    Definite byte strings only

.. function:: bool cbor_bytestring_is_definite(const cbor_item_t * item)
.. function:: bool cbor_bytestring_is_indefinite(const cbor_item_t * item)

Reading data
~~~~~~~~~~~~~

.. function:: unsigned char * cbor_bytestring_handle(const cbor_item_t * item)
	
	Returns a pointer to a contiguous area of memory containing :func:`cbor_bytestring_length` bytes.


.. function:: cbor_item_t ** cbor_bytestring_chunks_handle(const cbor_item_t * item)

    Indefinite byte strings only. Returns an array of chunks (definite byte strings)


.. function:: size_t cbor_bytestring_chunk_count(const cbor_item_t * item)

    Indefinite byte strings only. Returns the number of chunks

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: cbor_item_t * cbor_new_definite_bytestring()
.. function:: cbor_item_t * cbor_new_indefinite_bytestring()

Manipulating existing items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: cbor_bytestring_set_handle(cbor_item_t * item, unsigned char * data, size_t length)

    Attaches the ``item`` to the ``data`` of length ``length``. Ownership of the memory is transferred to *libcbor*. It will be deallocated when the item is deallocated. It is the user's responsibility to ensure that the area has been correctly allocated and may be safely deallocated using the specified deallocator (TODO link to custom malloc).


Building items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: cbor_item_t * cbor_bytestring_add_chunk(cbor_item_t * item, cbor_item_t * chunk)

    Returns the original pointer on success, ``NULL`` on failure. May :func:`realloc` the storage

.. function:: cbor_item_t * cbor_bytestring_delete_chunk(cbor_item_t * item, size_t index)

Transformations
~~~~~~~~~~~~~~~~~~~

.. function:: cbor_item_t * cbor_bytestring_concatenate(cbor_item_t * item)

    Create new definite length byte string by concatenation all chunks of an indefinite one.

