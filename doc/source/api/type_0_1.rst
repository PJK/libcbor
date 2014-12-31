Types 0 & 1 – Positive and negative integers
===============================================

*CBOR* has two types of integers – positive (which may be effectively regarded as unsigned), and negative. There are four possible widths for an integer – 1, 2, 4, or 8 bytes. These are represented by

.. type:: enum cbor_int_width

   +--------------------+
   | ``CBOR_INT_8``     |
   +--------------------+
   | ``CBOR_INT_16``    |
   +--------------------+
   | ``CBOR_INT_32``    |
   +--------------------+
   | ``CBOR_INT_64``    |
   +--------------------+

Type 0 - positive integers
--------------------------
==================================  =========================================
Corresponding :type:`cbor_type`     ``CBOR_TYPE_UINT``
Number of allocations               One per lifetime
Storage requirements                ``sizeof(cbor_item_t) + sizeof(uint*_t)``
==================================  =========================================

**Note:** once a positive integer has been created, its width *cannot* be changed.

Type 1 - negative integers
--------------------------
==================================  =========================================
Corresponding :type:`cbor_type`     ``CBOR_TYPE_NEGINT``
Number of allocations               One per lifetime
Storage requirements                ``sizeof(cbor_item_t) + sizeof(uint*_t)``
==================================  =========================================

**Note:** once a positive integer has been created, its width *cannot* be changed.

Type 0 & 1
-------------
Due to their largely similar semantics, the following functions can be used for both Type 0 and Type 1 items. One can convert between them freely using `the conversion functions <#dealing-with-signedness>`_.

Actual Type of the integer can be checked using :ref:`item types API <item-types>`.



An integer item is created with one of the four widths. Because integers' `storage is bundled together with the handle </internal#c.cbor_item_t.data>`_, the width cannot be changed over its lifetime.

Retrieving values
------------------------
.. function:: uint8_t cbor_get_uint8(const cbor_item_t * item)
.. function:: uint16_t cbor_get_uint16(const cbor_item_t * item)
.. function:: uint32_t cbor_get_uint32(const cbor_item_t * item)
.. function:: uint64_t cbor_get_uint64(const cbor_item_t * item)

Setting values
------------------------
.. function:: void cbor_set_uint8(cbor_item_t * item, uint8_t value)
.. function:: void cbor_set_uint16(cbor_item_t * item, uint16_t value)
.. function:: void cbor_set_uint32(cbor_item_t * item, uint32_t value)
.. function:: void cbor_set_uint64(cbor_item_t * item, uint64_t value)

Dealing with width
---------------------
.. function:: cbor_int_width cbor_int_get_width(const cbor_item_t * item)

Dealing with signedness
--------------------------
.. function:: void cbor_mark_uint(cbor_item_t * item)

	Mark the item to be interpreted as a positive/unsigned integer. This only changes the interpretation of the value, no the value itself.

.. function:: void cbor_mark_negint(cbor_item_t * item)

	Mark the item to be interpreted as a negative integer. This only changes the interpretation of the value, no the value itself.

Creating new items
------------------------
.. function:: cbor_item_t * cbor_new_int8()

	Create an uninitialized :type:`cbor_item_t` of :type:`cbor_type` ``CBOR_TYPE_UINT`` and :type:`cbor_int_width` ``CBOR_INT_8``.

.. function:: cbor_item_t * cbor_new_int16()

	Create an uninitialized :type:`cbor_item_t` of :type:`cbor_type` ``CBOR_TYPE_UINT`` and :type:`cbor_int_width` ``CBOR_INT_16``.

.. function:: cbor_item_t * cbor_new_int32()

	Create an uninitialized :type:`cbor_item_t` of :type:`cbor_type` ``CBOR_TYPE_UINT`` and :type:`cbor_int_width` ``CBOR_INT_32``.

.. function:: cbor_item_t * cbor_new_int64()

	Create an uninitialized :type:`cbor_item_t` of :type:`cbor_type` ``CBOR_TYPE_UINT`` and :type:`cbor_int_width` ``CBOR_INT_64``.
