Types 0 & 1 – Positive and negative integers
=============================

*CBOR* has two types of integers – positive (which may be effectively regarded as unsigned), and negative. There are four possible widths for an integer – 1, 2, 4, or 8 bytes. These are represented by

.. type:: enum cbor_int_width

   which consists of

   +--------------------+
   | ``CBOR_INT_8``     |
   +--------------------+
   | ``CBOR_INT_16``    |
   +--------------------+
   | ``CBOR_INT_32``    |
   +--------------------+
   | ``CBOR_INT_64``    |
   +--------------------+



Retrieving values
------------------------
.. function:: uint8_t cbor_get_uint8(cbor_item_t * item)
.. function:: uint16_t cbor_get_uint16(cbor_item_t * item)
.. function:: uint32_t cbor_get_uint32(cbor_item_t * item)
.. function:: uint64_t cbor_get_uint64(cbor_item_t * item)

Setting values
------------------------
.. function:: void cbor_set_uint8(cbor_item_t * item, uint8_t value)
.. function:: void cbor_set_uint16(cbor_item_t * item, uint16_t value)
.. function:: void cbor_set_uint32(cbor_item_t * item, uint32_t value)
.. function:: void cbor_set_uint64(cbor_item_t * item, uint64_t value)

Dealing with width
---------------------
.. function:: cbor_int_width cbor_int_get_width(cbor_item_t * item)

Dealing with signedness
--------------------------
.. function:: void cbor_mark_uint(cbor_item_t * item)
.. function:: void cbor_mark_negint(cbor_item_t * item)

Creating new items
------------------------
.. function:: cbor_item_t * cbor_new_int8()
.. function:: cbor_item_t * cbor_new_int16()
.. function:: cbor_item_t * cbor_new_int32()
.. function:: cbor_item_t * cbor_new_int64()