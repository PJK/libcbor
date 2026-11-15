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

   A capping allocator puts an absolute limit on the memory ``cbor_load`` uses. Without one,
   ``cbor_load`` uses memory proportional to the size of its input (see :doc:`decoding`), so limiting the
   input size also bounds it. A ``malloc`` wrapper that returns ``NULL`` above a chosen threshold causes
   ``cbor_load`` to return ``CBOR_ERR_MEMERROR`` cleanly once the threshold is reached. See
   ``examples/capped_alloc.c`` for a complete example.

.. code-block:: c

   /* Note: not thread-safe; guard allocated_bytes with a mutex if needed */
   #define MAX_TOTAL_SIZE (8 * 1024 * 1024)  /* 8 MiB */
   static size_t allocated_bytes = 0;

   static void* capping_malloc(size_t size) {
     if (size > MAX_TOTAL_SIZE - allocated_bytes) return NULL;
     void* ptr = malloc(size);
     if (ptr != NULL) allocated_bytes += size;
     return ptr;
   }

   /* ... matching capping_realloc and free; see examples/capped_alloc.c */

   /* Install before any cbor_load calls */
   cbor_set_allocs(capping_malloc, capping_realloc, capping_free);

.. doxygenfunction:: cbor_set_allocs


Reference counting
^^^^^^^^^^^^^^^^^^^^^

As CBOR items may require complex cleanups at the end of their lifetime, there is a reference counting mechanism in place. This also enables a very simple GC when integrating *libcbor* into a managed environment. Every item starts its life (by either explicit creation, or as a result of parsing) with reference count set to 1. When the refcount reaches zero, it will be destroyed.

Items containing nested items will be destroyed transitively — the refcount of every nested item will be decreased by one iteratively in constant stack space and with no additional allocations.

The destruction is synchronous and renders any pointers to items with refcount zero invalid immediately after calling :func:`cbor_decref`. See :doc:`/tutorial` (``examples/crash_course.c``) for a runnable walkthrough.

.. _ownership-conventions:

Ownership conventions across the API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. rst-class:: fixed-table

.. list-table::
   :header-rows: 1
   :widths: 28 32 40

   * - Operation kind
     - Functions
     - Reference-count and ownership effect
   * - **Creation, decoding, and copying**
     - ``cbor_new_*``, ``cbor_build_*``, :func:`cbor_load`,
       :func:`cbor_copy`, :func:`cbor_copy_definite`
     - Return a new item with ``refcount == 1`` (or ``NULL`` on failure).
       The caller owns the returned reference and must release it with
       :func:`cbor_decref`. (:func:`cbor_build_tag` also increments its
       tagged ``item``'s refcount by 1 on success.)
   * - **Container and tag mutators**
     - :func:`cbor_array_push`, :func:`cbor_array_set`,
       :func:`cbor_array_replace`, :func:`cbor_map_add`,
       :func:`cbor_tag_set_item`, :func:`cbor_string_add_chunk`,
       :func:`cbor_bytestring_add_chunk`
     - Share ownership by incrementing the inserted item's refcount by 1
       (both ``key`` and ``value`` in :func:`cbor_map_add`). Overwriting an
       existing slot (:func:`cbor_array_replace`, :func:`cbor_array_set` at
       ``index < size``, :func:`cbor_tag_set_item` when already set)
       decrements the displaced item's refcount by 1. On failure, no
       refcounts are modified.
   * - **Item getters**
     - :func:`cbor_array_get`, :func:`cbor_map_get`, :func:`cbor_tag_item`
     - Increment the returned item's refcount by 1 when non-``NULL``. The
       caller must release the returned reference with :func:`cbor_decref`.
   * - **Inspectors, scalar setters, serializers, and raw handles**
     - :func:`cbor_typeof`, ``cbor_isa_*``, ``cbor_is_*``, ``cbor_get_*``,
       ``cbor_set_*``, ``cbor_serialize*`` (except
       :func:`cbor_serialize_alloc`, see below),
       :func:`cbor_structurally_equal`, ``cbor_describe``,
       :func:`cbor_array_handle`, :func:`cbor_map_handle`,
       :func:`cbor_string_chunks_handle`,
       :func:`cbor_bytestring_chunks_handle`, :func:`cbor_string_handle`,
       :func:`cbor_bytestring_handle`
     - **Borrow** their item arguments without modifying any refcount.
       ``*_handle`` functions expose internal storage owned by the item
       (``NULL`` when empty), invalidated when the container reallocates or
       is destroyed; replacing child pointers in place requires manual
       :func:`cbor_incref` and :func:`cbor_decref` calls.
   * - **Raw byte-buffer transfers**
     - :func:`cbor_string_set_handle`, :func:`cbor_bytestring_set_handle`,
       :func:`cbor_serialize_alloc`
     - ``*_set_handle`` transfers ownership of a heap block (allocated via
       the allocator configured with :func:`cbor_set_allocs`) to *libcbor*
       without freeing any previously attached block.
       :func:`cbor_serialize_alloc` transfers a newly allocated buffer to
       the caller, who must free it with the matching ``free`` function.
       (In :doc:`streaming_decoding`, :type:`cbor_string_callback` receives
       a borrowed, non-null-terminated slice into ``source``.)

.. _refcount-pitfalls:

Common patterns and pitfalls
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Releasing references and dangling pointers**: :func:`cbor_decref` sets the
caller's pointer to ``NULL`` *only* when the item's reference count reaches
zero and the item is freed. When another owner (such as a container) still
holds a reference, the pointer remains non-``NULL`` and becomes a dangling
pointer once the last owner releases the item:

.. code-block:: c

   cbor_item_t *array = cbor_new_definite_array(1);
   cbor_item_t *elem = cbor_build_uint8(42);  /* elem refcount == 1 */
   cbor_array_push(array, elem);              /* elem refcount == 2 */

   cbor_decref(&elem);   /* elem refcount == 1; elem is NOT set to NULL */
   cbor_decref(&array);  /* array AND elem are freed; array == NULL, elem dangles */

**Nested construction and** :func:`cbor_move`: Passing a freshly built item
directly to a container mutator (``cbor_array_push(array, cbor_build_uint8(42))``)
leaks the caller's initial reference because ``cbor_array_push`` increments the
refcount to 2. :func:`cbor_move` decrements the refcount by 1 without
deallocating, so ``cbor_array_push(array, cbor_move(cbor_build_uint8(42)))``
leaves the element owned by the array alone. This concise form is used
throughout the README and ``examples/``, and is fine for code that treats
allocation failure as fatal.

.. warning::

   Code that must recover from allocation failures (for example, under a
   capping allocator installed with :func:`cbor_set_allocs`) should not use
   ``cbor_move(cbor_build_*(...))``:

   1. If ``cbor_build_*`` returns ``NULL``, ``cbor_move(NULL)`` returns
      ``NULL``, violating the non-``NULL`` precondition of container mutators.
   2. If the outer call fails (e.g., :func:`cbor_array_push`,
      :func:`cbor_map_add`, or :func:`cbor_build_tag` encounters an allocation
      failure or a full definite container), it leaves the moved item at
      ``refcount == 0`` without freeing it, leaking the item.

   Hold the child in a local variable, check for ``NULL``, and release your
   reference unconditionally with :func:`cbor_decref`:

   .. code-block:: c

      cbor_item_t *elem = cbor_build_uint8(42);
      if (elem == NULL) { /* handle OOM */ }
      bool pushed = cbor_array_push(array, elem);
      cbor_decref(&elem);  /* frees elem on push failure; drops ref to 1 on success */
      if (!pushed) { /* handle failure */ }

**Shared items vs. reference cycles**: An item may be referenced by multiple
containers (forming a directed acyclic graph); :func:`cbor_decref` decrements
each incoming edge independently and :func:`cbor_copy` deep-copies the graph
into an unshared tree. **Reference cycles** (such as pushing an array into
itself, directly or indirectly) are *not* supported: cyclic items will never
reach a reference count of zero and will cause unbounded recursion in
tree-walking routines (:func:`cbor_serialize`, :func:`cbor_copy`,
:func:`cbor_structurally_equal`, ``cbor_describe``).


API reference
~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_incref
.. doxygenfunction:: cbor_decref
.. doxygenfunction:: cbor_intermediate_decref
.. doxygenfunction:: cbor_refcount
.. doxygenfunction:: cbor_move
.. doxygenfunction:: cbor_copy
.. doxygenfunction:: cbor_copy_definite
