Decoding
=============================

The following diagram illustrates the relationship among different parts of libcbor from the decoding standpoint.

::

    ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    │                                                                                              │
    │                                      Client application                                      │
    │                                                                                              │
    │                                                 ┌────────────────────────────────────────────┘
    │                                                 │                     ↕
    │                                                 │ ┌──────────────────────────────────────────┐
    │                                                 │ │                                          │
    │                                                 │ │          Manipulation routines           │
    │                                                 │ │                                          │
    │           ┌─────────────────────────────────────┘ └──────────────────────────────────────────┘
    │           │     ↑    ↑                  ↑                              ↑
    │           │     │    │    ┌─────────────╫──────────┬───────────────────┴─┐
    │           │     │   CDS   │             ║          │                     │
    │           │     │    │   PDS            ║         PDS                   PDS
    │           │     ↓    ↓    ↓             ↓          ↓                     ↓
    │           │ ┌─────────────────┐   ┌────────────────────┐   ┌────────────────────────────┐
    │           │ │                 │   │                    │   │                            │
    │           │ │  Custom driver  │ ↔ │  Streaming driver  │ ↔ │       Default driver       │ ↔ CD
    │           │ │                 │   │                    │   │                            │
    └───────────┘ └─────────────────┘   └────────────────────┘   └────────────────────────────┘
          ↕                ↕                        ↕                           ↕
    ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    │                                                                                              │
    │                            Stateless event─driven decoder                                    │
    │                                                                                              │
    └──────────────────────────────────────────────────────────────────────────────────────────────┘

                  (PDS = Provided Data Structures, CDS = Custom Data Structures)

This section will deal with the API that is labeled as the "Default driver" in the diagram. That is, routines that
decode complete libcbor data items

.. warning::

   ``cbor_load`` pre-allocates storage for definite-length collections (arrays
   and maps) whose size is declared in the CBOR header. A crafted input can
   declare a collection of up to 2\ :sup:`64`\−1 elements in as few as 9 bytes,
   triggering an allocation of many gigabytes before any element data is
   read. ``malloc`` will normally refuse such a request and ``cbor_load``
   will return ``CBOR_ERR_MEMERROR``, but on platforms with memory overcommit
   (Linux by default) the allocation may appear to succeed.

   Applications that parse untrusted CBOR data should install a capping
   allocator via :func:`cbor_set_allocs` to bound the total memory that
   ``cbor_load`` may consume. Alternatively, use the streaming decoder
   (:doc:`streaming_decoding`) which gives the application full control over
   memory allocation for each decoded item.

.. doxygenfunction:: cbor_load

Associated data structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenenum:: cbor_error_code

.. doxygenstruct:: cbor_load_result
    :members:

.. doxygenstruct:: cbor_error
    :members:

