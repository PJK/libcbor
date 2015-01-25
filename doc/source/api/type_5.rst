Type 5 – Maps
=============================

TODO

Streaming maps
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please refer to :doc:`/streaming`.

Getting metadata
~~~~~~~~~~~~~~~~~
.. function:: bool cbor_map_is_definite(cbor_item_t * item)
.. function:: bool cbor_map_is_indefinite(cbor_item_t * item)
.. function:: size_t cbor_map_size(cbor_item_t * item)


Reading data
~~~~~~~~~~~~~

.. function:: struct cbor_map_iterator cbor_map_begin(cbor_item_t * item)
.. function:: bool cbor_map_iterator_end(struct cbor_map_iterator * iter)
.. function:: void cbor_map_iterator_next(struct cbor_map_iterator * iter)

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TODO

.. function:: cbor_item_t * cbor_new_map()


Building items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. function:: struct cbor_map_iterator cbor_map_add(cbor_item_t * item, struct cbor_pair pair)
.. function:: void cbor_map_delete(struct cbor_map_iterator * iter)
