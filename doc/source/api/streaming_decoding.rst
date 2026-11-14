Streaming Decoding
=============================

*libcbor* exposes a stateless decoder that reads a stream of input bytes from a buffer and invokes user-provided callbacks as it decodes the input:

.. doxygenfunction:: cbor_stream_decode

For example, when :func:`cbor_stream_decode` encounters a 1B unsigned integer, it will invoke the function pointer stored in ``cbor_callbacks.uint8``.
Complete usage example: `examples/streaming_parser.c <https://github.com/PJK/libcbor/blob/master/examples/streaming_parser.c>`_


Decoding results and buffer management
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each call to :func:`cbor_stream_decode` decodes a single token — one primitive
value, definite-length string or bytestring, collection or tag header
(``array_start``, ``indef_array_start``, ``map_start``, ``indef_map_start``,
``tag``), or ``break`` stop code (``indef_break``) — and returns a
:type:`cbor_decoder_result`:

- ``CBOR_DECODER_FINISHED``: one callback was invoked; advance the input buffer
  by ``result.read`` bytes and call :func:`cbor_stream_decode` again to
  continue.
- ``CBOR_DECODER_NEDATA``: the buffer ends mid-token. If the complete input is
  already in memory (as in ``examples/streaming_parser.c``), the input is
  truncated. When reading from a stream, accumulate at least
  ``result.required`` bytes starting at the current position and retry the
  call.
- ``CBOR_DECODER_ERROR``: the token header is malformed (e.g. a reserved
  additional information value or an ill-formed two-byte simple value).

Because :func:`cbor_stream_decode` retains no state across calls, multi-item
structural rules — tracking nested definite collections and tags, matching
``indef_break`` to an open indefinite item, checking that indefinite-length
string chunks are definite strings of the matching major type, and ensuring
indefinite-length maps contain an even number of items — are the caller's
responsibility, alongside any semantic validity checks (see
:ref:`decoder-validity`).

.. warning::

   Lengths declared in CBOR headers come directly from the input and must be
   treated as untrusted:

   - **Resizing buffers on** ``CBOR_DECODER_NEDATA``: ``result.required`` is
     computed from the declared item length and may be arbitrarily large up to
     ``SIZE_MAX`` (at which it saturates if the header size plus payload length
     overflows ``size_t``). Callers that grow an input buffer using
     ``result.required`` must check it against an application-defined limit
     *before* allocating or performing arithmetic on it (for example,
     ``result.required + 1`` wraps around when ``result.required == SIZE_MAX``).
   - **Callback length parameters**: ``string`` and ``byte_string`` callbacks
     are invoked only after all ``length`` payload bytes have been verified to
     be present in ``source``. By contrast, ``array_start`` and ``map_start``
     callbacks receive the raw 64-bit element or pair count from the header
     before any elements are read; callbacks that preallocate storage must
     bound this count (and verify it fits in ``size_t`` on platforms where
     ``size_t`` is narrower than 64 bits).

.. doxygenenum:: cbor_decoder_status

.. doxygenstruct:: cbor_decoder_result
    :members:


Callbacks
~~~~~~~~~

The callbacks are defined by

.. doxygenstruct:: cbor_callbacks
    :members:

When building custom sets of callbacks, feel free to start from

.. doxygenvariable:: cbor_empty_callbacks


Handling failures in callbacks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Callbacks receive no return value, so there is no built-in channel to signal
failure back to :func:`cbor_stream_decode`. Any error that occurs inside a
callback — including allocation failures, validation errors, or application-level
rejections — must be tracked by the callback itself, typically via a flag in the
``context`` struct passed to :func:`cbor_stream_decode`:

.. code-block:: c

    struct my_context {
        bool failed;
        /* ... */
    };

    void my_string_callback(void *context, cbor_data data, uint64_t length) {
        struct my_context *ctx = context;
        if (length > MAX_ALLOWED) {
            ctx->failed = true;
            return;
        }
        char *copy = malloc(length);
        if (copy == NULL) {
            ctx->failed = true;
            return;
        }
        /* ... */
    }

After each call to :func:`cbor_stream_decode`, check the flag before
continuing. Note that :func:`cbor_load` handles allocation failures internally —
the ``CBOR_ERR_MEMERROR`` result code is set when a builder callback fails to
allocate memory.


Callback types definition
~~~~~~~~~~~~~~~~~~~~~~~~~~~~


.. doxygentypedef:: cbor_int8_callback
.. doxygentypedef:: cbor_int16_callback
.. doxygentypedef:: cbor_int32_callback
.. doxygentypedef:: cbor_int64_callback
.. doxygentypedef:: cbor_simple_callback
.. doxygentypedef:: cbor_string_callback
.. doxygentypedef:: cbor_collection_callback
.. doxygentypedef:: cbor_float_callback
.. doxygentypedef:: cbor_double_callback
.. doxygentypedef:: cbor_bool_callback
