API
=======
The *libcbor* API closely follows the semantics outlined by `CBOR standard <http://tools.ietf.org/html/rfc7049>`_. This part of the documentation provides a short overview of the CBOR constructs, as well as a general introduction to the *libcbor* API. Remaining reference can be found in the following files structured by data types

.. toctree::

   api/type_0_1
   api/type_2
   api/type_3
   api/type_4
   api/type_5
   api/type_6
   api/type_7

A quick CBOR primer
--------------------------

TODO

The API
-----------------

The API is centered around :type:`cbor_item_t`, a generic handle for any CBOR item. There are functions to

 - create items,
 - set items' data,
 - parse serialized data into items,
 - manage, move, and links item together.

The single most important thing to keep in mind is: :type:`cbor_item_t` **is an opaque type and should only be manipulated using the appropriate functions!** Think of it as an object.

.. type:: cbor_item_t

Memory management
~~~~~~~~~~~~~~~~~

Due to the nature of its domain *libcbor* will need to work with heap memory. It will only use ``malloc`` and ``free``.

TODO custom malloc

Reference counting
~~~~~~~~~~~~~~~~~~

As CBOR items may require complex cleanups at the end of their lifetime, there is a reference counting mechanism in place. This also enables very simple GC when integrating *libcbor* into managed environment. Every item starts its life (by either explicit creation, or as a result of parsing) with reference count set to 1. When the refcount reaches zero, it will be destroyed.

Items containing nested items will be destroyed recursively - refcount of every nested item will be decreased by one.

The destruction is synchronous and might possibly render any pointers to items with refcount zero invalid immediately after calling the ``decref``.


.. function:: void cbor_incref(cbor_item_t * item)

	Increment the reference counter.

.. function:: void cbor_decref(cbor_item_t ** item)

	Decrement the reference counter. Deallocate the item if it reaches zero and recursively decref all dependent items.

.. _item-types:

Item types
~~~~~~~~~~~

A :type:`cbor_item_t` can be probed for its type using these functions

.. function:: bool cbor_isa_uint(const cbor_item_t * item)
.. function:: bool cbor_isa_negint(const cbor_item_t * item)
.. function:: bool cbor_isa_bytestring(const cbor_item_t * item)
.. function:: bool cbor_isa_string(const cbor_item_t * item)
.. function:: bool cbor_isa_array(const cbor_item_t * item)
.. function:: bool cbor_isa_map(const cbor_item_t * item)
.. function:: bool cbor_isa_tag(const cbor_item_t * item)
.. function:: bool cbor_isa_float_ctrl(const cbor_item_t * item)