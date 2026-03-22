Streaming Decoding
=============================

*libcbor* exposes a stateless decoder that reads a stream of input bytes from a buffer and invokes user-provided callbacks as it decodes the input:

.. doxygenfunction:: cbor_stream_decode

For example, when :func:`cbor_stream_decode` encounters a 1B unsigned integer, it will invoke the function pointer stored in ``cbor_callbacks.uint8``.
Complete usage example: `examples/streaming_parser.c <https://github.com/PJK/libcbor/blob/master/examples/streaming_parser.c>`_

The callbacks are defined by

.. doxygenstruct:: cbor_callbacks
    :members:

When building custom sets of callbacks, feel free to start from

.. doxygenvariable:: cbor_empty_callbacks


Handling allocation failures in callbacks
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Callbacks receive no return value, so there is no built-in channel to signal
allocation failure back to :func:`cbor_stream_decode`. When a callback allocates
memory and the allocation fails, the callback must track the failure itself —
typically via a flag in the ``context`` struct passed to
:func:`cbor_stream_decode`:

.. code-block:: c

    struct my_context {
        bool allocation_failed;
        /* ... */
    };

    void my_string_callback(void *context, cbor_data data, uint64_t length) {
        struct my_context *ctx = context;
        char *copy = malloc(length);
        if (copy == NULL) {
            ctx->allocation_failed = true;
            return;
        }
        /* ... */
    }

After each call to :func:`cbor_stream_decode`, check the flag before
continuing. Note that :func:`cbor_load` handles this internally — the
``CBOR_ERR_MEMERROR`` result code is set when a builder callback fails to
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
