Memory management and reference counting
===============================================

Due to the nature of its domain, *libcbor* will need to work with heap memory. The stateless decoder and encoder doesn't allocate any memory.

If you have specific requirements, you should consider rolling your own driver for the stateless API.

Using custom allocator
^^^^^^^^^^^^^^^^^^^^^^^^

*libcbor* gives you the ability to provide your own implementations of ``malloc``, ``realloc``, and ``free``.
This is useful if you are using a custom allocator throughout your application, or if you want to enforce
memory limits when parsing untrusted data.

.. note::

   A capping allocator is the recommended way to bound memory consumption in ``cbor_load``. Because
   definite-length arrays and maps pre-allocate storage for the declared number of elements, a crafted
   input can trigger a very large allocation before any element data is read. A ``malloc`` wrapper that
   returns ``NULL`` above a chosen threshold causes ``cbor_load`` to return ``CBOR_ERR_MEMERROR`` cleanly
   rather than attempting the allocation.

.. code-block:: c

	cbor_set_allocs(malloc, realloc, free);

.. doxygenfunction:: cbor_set_allocs


Reference counting
^^^^^^^^^^^^^^^^^^^^^

As CBOR items may require complex cleanups at the end of their lifetime, there is a reference counting mechanism in place. This also enables a very simple GC when integrating *libcbor* into a managed environment. Every item starts its life (by either explicit creation, or as a result of parsing) with reference count set to 1. When the refcount reaches zero, it will be destroyed.

Items containing nested items will be destroyed recursively - the refcount of every nested item will be decreased by one.

The destruction is synchronous and renders any pointers to items with refcount zero invalid immediately after calling :func:`cbor_decref`.


.. doxygenfunction:: cbor_incref
.. doxygenfunction:: cbor_decref
.. doxygenfunction:: cbor_intermediate_decref
.. doxygenfunction:: cbor_refcount
.. doxygenfunction:: cbor_move
.. doxygenfunction:: cbor_copy
.. doxygenfunction:: cbor_copy_definite
