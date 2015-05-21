Type 7 – Floats & control tokens
=================================

This type combines two completely unrelated types of items -- floating point numbers and special values such as true, false, null, etc. We refer to these special values as 'control values' or 'ctrls' for short throughout the code.

Just like integers, they have different possible width (resulting in different value ranges and precisions).

.. doxygenenum:: cbor_float_width

==================================  =========================================
Corresponding :type:`cbor_type`     ``CBOR_TYPE_FLOAT_CTRL``
Number of allocations               One per lifetime
Storage requirements                ``sizeof(cbor_item_t) + 1/4/8``
==================================  =========================================

Getting metadata
~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_float_ctrl_is_ctrl
.. doxygenfunction:: cbor_float_get_width
.. doxygenfunction:: cbor_ctrl_is_bool

Reading data
~~~~~~~~~~~~~

.. doxygenfunction:: cbor_float_get_float2
.. doxygenfunction:: cbor_float_get_float4
.. doxygenfunction:: cbor_float_get_float8
.. doxygenfunction:: cbor_float_get_float
.. doxygenfunction:: cbor_ctrl_value
.. doxygenfunction::

Creating new items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_new_ctrl
.. doxygenfunction:: cbor_new_float2
.. doxygenfunction:: cbor_new_float4
.. doxygenfunction:: cbor_new_float8
.. doxygenfunction:: cbor_new_null
.. doxygenfunction:: cbor_new_undef


Building items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_build_bool
.. doxygenfunction:: cbor_build_ctrl
.. doxygenfunction:: cbor_build_float2
.. doxygenfunction:: cbor_build_float4
.. doxygenfunction:: cbor_build_float8


Manipulating existing items
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_set_ctrl
.. doxygenfunction:: cbor_set_float2
.. doxygenfunction:: cbor_set_float4
.. doxygenfunction:: cbor_set_float8

