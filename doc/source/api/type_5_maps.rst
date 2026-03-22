Type 5 – Maps
=============================

CBOR maps are the plain old associative maps similar JSON objects or Python dictionaries.

Definite maps have a fixed size which is stored in the header, whereas indefinite maps do not and are terminated by a special "break" byte instead.

Maps are explicitly created or decoded as definite or indefinite and will be encoded using the corresponding wire representation, regardless of whether the actual size is known at the time of encoding.

.. note::

  Indefinite maps can be conveniently used with streaming :doc:`decoding <streaming_decoding>` and :doc:`encoding <streaming_encoding>`.
  Keys and values can simply be output one by one, alternating keys and values.

.. warning:: Any CBOR data item is a legal map key (not just strings).

==================================  =====================================================================================
Corresponding :type:`cbor_type`     ``CBOR_TYPE_MAP``
Number of allocations (definite)    Two plus any manipulations with the data
Number of allocations (indefinite)  Two plus logarithmically many
                                    reallocations relative to additions
Storage requirements (definite)     ``sizeof(cbor_pair) * size + sizeof(cbor_item_t)``
Storage requirements (indefinite)   ``<= sizeof(cbor_item_t) + sizeof(cbor_pair) * size * BUFFER_GROWTH``
==================================  =====================================================================================

Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    0xbf        Start indefinite map (represents {1: 2})
        0x01        Unsigned integer 1 (key)
        0x02        Unsigned integer 2 (value)
        0xff        "Break" control token

::

    0xa0        Map of size 0

Getting metadata
~~~~~~~~~~~~~~~~~
.. doxygenfunction:: cbor_map_size
.. doxygenfunction:: cbor_map_allocated
.. doxygenfunction:: cbor_map_is_definite
.. doxygenfunction:: cbor_map_is_indefinite

Reading data
~~~~~~~~~~~~~

.. doxygenfunction:: cbor_map_handle
.. doxygenfunction:: cbor_map_get

.. note::

   The equality function passed to :func:`cbor_map_get` is intentionally
   parameterized. Most applications constrain their map keys to a single
   type (e.g., always text strings, or always small integers), so a
   type-specific comparator can be both simpler and faster than a fully
   generic one.  The parameter also lets callers choose between structural
   equality (:func:`cbor_structurally_equal`) and any data-model or
   application-level semantics they need — the library does not need to
   anticipate every use case.

   Examples of equality functions that can be passed:

   * :func:`cbor_structurally_equal` — encoding-level identity; encoding
     width, definite/indefinite, and map entry order all count.
   * A custom integer comparator that ignores encoding width, so that
     ``uint8(1)`` and ``uint64(1)`` are treated as the same key.
   * A text-string comparator that skips the type check, useful when the
     protocol guarantees that all keys are text strings.

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_new_definite_map
.. doxygenfunction:: cbor_new_indefinite_map


Modifying items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_map_add
