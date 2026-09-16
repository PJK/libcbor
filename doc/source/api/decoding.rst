Decoding
=============================

libcbor provides two decoding interfaces:

.. rst-class:: fixed-table

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

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
     - Caller's responsibility. Nested structures
       (e.g. arrays containing maps) require the
       caller to maintain an explicit stack to track
       the current depth and accumulate children.
   * - **Memory**
     - Allocates the ``cbor_item_t`` tree.
       Free with :func:`cbor_decref`.
     - No library allocations.
   * - **Best for**
     - - Trusted or size-bounded inputs.
       - General data manipulation and inspection.
       - Simple use cases.
     - - Large or unbounded inputs.
       - Memory-constrained environments.
       - Mapping to custom data structures.
       - Known, restricted schemas.

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

.. _decoder-validity:

Well-formedness and validity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

`RFC 8949 <https://www.rfc-editor.org/rfc/rfc8949#section-1.2>`_ distinguishes
two levels of correctness for a CBOR data item:

- **Well-formed**: the item follows the syntactic structure of CBOR — the
  initial bytes, argument, and any implied byte strings or nested data
  items are all present and consistent
  (`Section 3 <https://www.rfc-editor.org/rfc/rfc8949#section-3>`_).
- **Valid**: the item is well-formed *and* satisfies the semantic
  restrictions of
  `Section 5.3 <https://www.rfc-editor.org/rfc/rfc8949#section-5.3>`_,
  e.g. text strings are valid UTF-8, maps have no duplicate keys, and
  tag content has the type the tag requires.

Both libcbor decoders — :func:`cbor_load` and :func:`cbor_stream_decode` —
**check well-formedness only**. Any well-formed input is decoded successfully;
validity is left to the application, which is the only party that knows the
protocol's requirements
(`Section 5.4 <https://www.rfc-editor.org/rfc/rfc8949#section-5.4>`_).
Concretely, the decoders will accept:

.. rst-class:: fixed-table

.. list-table::
   :header-rows: 1
   :widths: 30 30 40

   * - Input
     - RFC 8949
     - How to check
   * - Text string that is not valid UTF-8,
       e.g. ``0x62 0xC0 0xAE``
     - `5.3.1 <https://www.rfc-editor.org/rfc/rfc8949#section-5.3.1>`_
     - :func:`cbor_string_codepoint_count` returns ``0``;
       see :doc:`type_3_strings`
   * - Map with duplicate keys,
       e.g. ``{1: 2, 1: 3}``
     - `5.3.1 <https://www.rfc-editor.org/rfc/rfc8949#section-5.3.1>`_
     - Iterate :func:`cbor_map_handle` and compare keys with
       :func:`cbor_structurally_equal`; see :doc:`type_5_maps`
   * - Tag whose content has an inadmissible type,
       e.g. tag 1 (epoch date) applied to a map
     - `5.3.2 <https://www.rfc-editor.org/rfc/rfc8949#section-5.3.2>`_
     - Inspect :func:`cbor_tag_value` and :func:`cbor_tag_item`;
       see :doc:`type_6_tags`

Inputs that are not well-formed — truncated data, reserved additional
information values, unexpected ``break`` codes, indefinite-length strings with
chunks of the wrong type, etc. — are rejected with a :type:`cbor_error_code`
describing the problem.

Note that the terminology in the CBOR working group's
`test vectors <https://github.com/cbor-wg/cbor-test-vectors>`_ and similar
suites does not always make this distinction. Test cases that expect a
decoder to *fail* on well-formed but invalid input are not applicable to
libcbor's decoders as designed.

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

