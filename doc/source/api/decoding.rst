Decoding
=============================

libcbor provides two decoding interfaces:

.. list-table::
   :header-rows: 1
   :widths: 20 40 40
   :width: 100%

   * -
     - Default
     - Streaming
   * - **Function**
     - :func:`cbor_load`
     - :func:`cbor_stream_decode`
   * - **Returns**
     - A fully-built ``cbor_item_t`` tree.
     - Nothing. Fires a callback for each decoded value.
   * - **Callbacks**
     - None. Results available after the call returns.
     - One callback per primitive, string chunk, or
       collection boundary (e.g. ``array_start``,
       ``uint8``, ``string_chunk``).
   * - **Nesting**
     - Handled by the library.
     - Caller's responsibility.
   * - **Memory**
     - Allocates the ``cbor_item_t`` tree.
       Free with :func:`cbor_decref`.
     - No library allocations.
   * - **Best for**
     - Most applications.
     - Large inputs, memory-constrained environments,
       or mapping directly to custom data structures.

:func:`cbor_load` is implemented on top of :func:`cbor_stream_decode` — it
installs its own internal callbacks that build the ``cbor_item_t`` tree as
events arrive.

.. graphviz::
   :align: center

   digraph decoding {
       graph [rankdir=TB, nodesep=1.4, ranksep=0.8, fontname="Helvetica"]
       node  [shape=box, style="filled,rounded", fontname="Helvetica", margin="0.3,0.15"]
       edge  [color="#555555", fontname="Helvetica", fontsize=10]

       client    [label="Client application",                   fillcolor="#AED6F1"]
       cbor_load [label="cbor_load",                            fillcolor="#FAD7A0"]
       streaming [label="cbor_stream_decode",                   fillcolor="#FAD7A0"]
       item      [label="cbor_item_t",                          fillcolor="#A9DFBF"]
       manip     [label="Manipulation routines\n(cbor_item_t API)", fillcolor="#D2B4DE"]

       { rank=same; cbor_load; streaming }
       { rank=same; item; manip }

       client -> cbor_load [label=" bytes "]
       client -> streaming [label=" bytes + callbacks "]

       cbor_load -> streaming [label=" internal callbacks ",
                               style=dashed, constraint=false]
       cbor_load -> item

       item:e -> manip:w [style=dashed, dir=both, constraint=false,
                          label=" use items "]
   }

This section covers the **Default driver** — :func:`cbor_load` and related
routines that decode a complete CBOR input into a ``cbor_item_t`` tree in one
call.

.. warning::

   ``cbor_load`` allocates memory sized by lengths declared in the CBOR header
   before reading the corresponding data:

   - **Definite-length arrays and maps** — storage for the declared element count.
   - **Definite-length strings and bytestrings** — a buffer for the declared byte length.

   All of these lengths are encoded as a 64-bit integer, so ``cbor_load`` may
   attempt an allocation of up to 2\ :sup:`64`\−1 bytes before any payload data
   is read. ``malloc`` will normally refuse such a request and ``cbor_load``
   will return ``CBOR_ERR_MEMERROR``, but on platforms with memory overcommit
   (Linux by default) the allocation may silently appear to succeed.

   Mitigations:

   - Install a capping allocator via :func:`cbor_set_allocs` to bound total
     memory consumption (see ``examples/capped_alloc.c`` for a self-contained
     example).
   - Use the streaming decoder (:doc:`streaming_decoding`), which gives the
     application full control over memory allocation for each decoded item.

.. doxygenfunction:: cbor_load

Associated data structures
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenenum:: cbor_error_code

.. doxygenstruct:: cbor_load_result
    :members:

.. doxygenstruct:: cbor_error
    :members:

