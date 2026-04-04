Decoding
=============================

libcbor provides two decoding interfaces:

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * -
     - Default (:func:`cbor_load`)
     - Streaming (:func:`cbor_stream_decode`)
   * - **Returns**
     - A fully-built ``cbor_item_t`` tree.
     - Nothing. Fires callbacks as items are decoded.
   * - **Callbacks**
     - None. Results available after the call returns.
     - One callback per primitive, string chunk, or collection boundary
       (e.g. ``array_start``, ``uint8``, ``string_chunk``).
   * - **Nesting**
     - Handled by the library.
     - Caller's responsibility.
   * - **Memory**
     - Allocates the ``cbor_item_t`` tree. Free with :func:`cbor_decref`.
     - No library allocations.
   * - **Best for**
     - Most applications.
     - Large inputs, memory-constrained environments, or mapping directly
       to custom data structures.

:func:`cbor_load` is implemented on top of :func:`cbor_stream_decode` — it
installs its own internal callbacks that build the ``cbor_item_t`` tree as
events arrive.

.. graphviz::
   :align: center

   digraph decoding {
       graph [rankdir=TB, nodesep=1.0, ranksep=0.8, fontname="Helvetica"]
       node  [shape=box, style="filled,rounded", fontname="Helvetica", margin="0.3,0.15"]
       edge  [color="#555555", fontname="Helvetica", fontsize=10]

       client    [label="Client application",              fillcolor="#AED6F1"]
       manip     [label="Manipulation routines\n(cbor_item_t API)", fillcolor="#D2B4DE"]
       stateless [label="Stateless event-driven decoder",  fillcolor="#A9DFBF"]

       subgraph cluster_drivers {
           label="Decoder interface — choose one"
           fontname="Helvetica"
           fontsize=11
           style=dashed
           color="#888888"
           node [fillcolor="#FAD7A0"]
           custom    [label="Custom driver\n(client-defined callbacks)"]
           streaming [label="Streaming driver\n(event callbacks → cbor_item_t)"]
           default   [label="Default driver\n(cbor_load → cbor_item_t)"]
       }

       { rank=same; client; manip }

       client -> custom
       client -> streaming
       client -> default

       custom    -> stateless
       streaming -> stateless
       default   -> stateless

       client:e -> manip:w [style=dashed, dir=both, constraint=false,
                            label="use decoded items"]
   }

This section covers the **Default driver** — :func:`cbor_load` and related
routines that decode a complete CBOR input into a ``cbor_item_t`` tree in one
call.

.. warning::

   ``cbor_load`` pre-allocates storage for definite-length collections (arrays
   and maps) sized by the element count declared in the CBOR header. The
   declared count can be up to 2\ :sup:`64`\−1, so ``cbor_load`` will attempt
   to allocate whatever the header says before reading any element data.
   ``malloc`` will normally refuse an unreasonably large request and
   ``cbor_load`` will return ``CBOR_ERR_MEMERROR``, but on platforms with
   memory overcommit (Linux by default) the allocation may appear to succeed.

   Applications that parse untrusted CBOR data should install a capping
   allocator via :func:`cbor_set_allocs` to bound the total memory that
   ``cbor_load`` may consume (see ``examples/capped_alloc.c`` for a
   self-contained example). Alternatively, use the streaming decoder
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

