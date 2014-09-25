Streaming & indefinite items
=============================

CBOR :doc:`strings <api/type_2>`, :doc:`byte strings <api/type_3>`, :doc:`arrays <api/type_4>`, and :doc:`maps <api/type_5>` can be encoded as *indefinite*, meaning their length or size is not specified. Instead, they are divided into *chunks* (:doc:`strings <api/type_2>`, :doc:`byte strings <api/type_3>`), or explicitly terminated (:doc:`arrays <api/type_4>`, :doc:`maps <api/type_5>`).

This is one of the most important (and due to poor implementations, underutilized) features of CBOR. It enables low-overhead streaming just about anywhere without dealing with channels or pub/sub mechanism. It is, however, important to recognize that CBOR streaming is not a substitute for  Websockets [#]_ and similar technologies.

.. [#] :RFC:`6455`

Streaming decoder
----------------------------------------

At the very heart of *libcbor* lies a decoder that *incrementally* decodes the input and invokes appropriate (possibly user-defined) callbacks:

.. function:: struct cbor_decoder_result cbor_stream_decode(cbor_data source, size_t source_size, const struct cbor_callbacks * callbacks, void * context)

    ``source`` and ``source_size`` describe the input buffer.

    One or more of ``callbacks`` will be invoked when an item is decoded.

    In order to facilitate the notion of *context*, ``context`` will be passed to any invoked callbacks. For an example on how to use this, please see the implementation of :func:`cbor_load`.

.. type:: enum cbor_decoder_status

    .. member:: CBOR_DECODER_FINISHED

        Successfully read an item


    .. member:: CBOR_DECODER_NEDATA

        There's not enough data to decode the whole item


    .. member:: CBOR_DECODER_EBUFFER

        The supplied buffer was empty


    .. member:: CBOR_DECODER_ERROR

        An error - malformated input


.. type:: struct cbor_decoder_result

    .. member:: size_t read

        Number of bytes decoded (they can be safely disposed)

    .. member:: enum cbor_decoder_status status



.. type:: struct cbor_callbacks

    .. member:: cbor_int8_callback uint8
    .. member:: cbor_int16_callback uint16
    .. member:: cbor_int32_callback uint32
    .. member:: cbor_int64_callback uint64

    .. member:: cbor_int8_callback negint8
    .. member:: cbor_int16_callback negint16
    .. member:: cbor_int32_callback negint32
    .. member:: cbor_int64_callback negint64

    .. member:: cbor_string_callback byte_string
    .. member:: cbor_simple_callback byte_string_start

    .. member:: cbor_string_callback string
    .. member:: cbor_simple_callback string_start

    .. member:: cbor_collection_callback array_start
    .. member:: cbor_simple_callback indef_array_start

    .. member:: cbor_collection_callback map_start
    .. member:: cbor_simple_callback indef_map_start


    .. member:: cbor_double_callback float2
    .. member:: cbor_float_callback float4
    .. member:: cbor_double_callback float8
    .. member:: cbor_simple_callback undefined
    .. member:: cbor_simple_callback null
    .. member:: cbor_bool_callback boolean

    .. member:: cbor_simple_callback indef_break

Streaming encoder
----------------------------------------
TODO
