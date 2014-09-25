Type 4 – Arrays
=============================

CBOR arrays, just like :doc:`byte strings <type_2>` and :doc:`strings <type_3>`, can be encoded either as definite, or as indefinite.

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    0x9f        Start indefinite array
        0x01        Unsigned integer 1
        0xff        "Break" control token

::

    0x9f        Start array, 1B length follows
    0x20        Unsigned integer 32
        ...        32 items follow

Streaming indefinite arrays
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please refer to :doc:`/streaming`.

Getting metadata
~~~~~~~~~~~~~~~~~
.. function:: size_t cbor_array_get_size(cbor_item_t * item)
.. function:: bool cbor_array_is_definite(cbor_item_t * item)
.. function:: bool cbor_array_is_indefinite(cbor_item_t * item)

Reading data
~~~~~~~~~~~~~
.. function:: cbor_item_t ** cbor_array_handle(cbor_item_t * item)

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. function:: cbor_item_t * cbor_new_definite_array(size_t)
.. function:: cbor_item_t * cbor_new_indefinite_array()


Building items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. function:: cbor_item_t * cbor_array_push(cbor_item_t * array, cbor_item_t * pushee)
