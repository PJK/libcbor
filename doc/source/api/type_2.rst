Type 2 – Byte strings
=============================

CBOR byte strings are just (ordered) series of bytes without further interpretation. Byte string's length may or may not be known during encoding. These two kinds of byte strings can be distinguished using :func:`cbor_bytestring_is_definite` and :func:`cbor_bytestring_is_indefinite` respectively.

In case a byte string is indefinite, it is encoded as a series of definite byte strings. These are called "chunks". For example, the encoded item

::

    0xf5	    Start indefinite byte string
	0x41	    Byte string (1B long)
	    0x00
	0x41	    Byte string (1B long)
	    0xff
	0xff	    "Break" control token

represents two bytes, ``0x00`` and ``0xff``. This on one hand enables streaming messages even before they are fully generated, but on the other hand it adds more complexity to the client code.

Reading indefinite byte strings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The first call to :func:`cbor_load` will return an item that represents the indefinite string. This item will keep a reference to up to **one** chunk.

To attempt loading a chunk, call :func:`cbor_bytestring_read_chunk`. On success, the chunk can be retrieved using :func:`cbor_bytestring_get_chunk`. **It is client's responsibility to** :func:`cbor_decref` **any loaded chunks, even if they don't intend to use them!** Client code should always check whether the returned chunk is an actual bytestring, or a Break (TODO).

Once a Break is encountered, the byte string has been fully read and the original item can be destroyed safely.

Generating indefinite byte strings
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When generating indefinite items, the client code has to take most of the responsibility for controlling the encoding, since items cannot be fully built before the encoding starts.

The first step is to initialize your item using :func:`cbor_new_indefinite_bytestring` and encode it (TODO).

Then you can start building chunks using :func:`cbor_new_definite_bytestring` and :func:`cbor_bytestring_set_handle`. Encode them so that they follow the initial indefinite item. (TODO)

Finally add a Break(TODO).


Getting metadata
~~~~~~~~~~~~~~~~~

.. function:: size_t cbor_bytestring_length(const cbor_item_t * item)

.. function:: bool cbor_bytestring_is_definite(const cbor_item_t * item)
.. function:: bool cbor_bytestring_is_indefinite(const cbor_item_t * item)

Reading data
~~~~~~~~~~~~~

.. function:: unsigned char * cbor_bytestring_handle(const cbor_item_t * item)
	
	Returns a pointer to a contiguous area of memory containing :func:`cbor_bytestring_length` bytes.

.. function:: cbor_item_t * cbor_bytestring_get_chunk(const cbor_item_t * item)

	Returns the current chunk. Indefinite byte strings only.

.. function:: void cbor_bytestring_read_chunk(cbor_item_t * item, const unsigned char * source, size_t source_size, struct cbor_load_result * result)

	Loads next chunk. Indefinite byte strings only.

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. function:: cbor_item_t * cbor_new_definite_bytestring()
.. function:: cbor_item_t * cbor_new_indefinite_bytestring()

Building items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. function:: void cbor_bytestring_set_handle(cbor_item_t * item, unsigned char * data, size_t length)
