Type 7 – Floats & control tokens
=================================

This type combines two completely unrelated types of items -- floating point numbers and special values such as true, false, null, etc. We refer to these special values as 'control values' or 'ctrls' for short throughout the code.

Simple values
~~~~~~~~~~~~~~

`RFC 8949 Section 3.3 <https://www.rfc-editor.org/rfc/rfc8949#section-3.3>`_ defines
simple values 0 to 255, of which only ``false`` (20), ``true`` (21), ``null`` (22),
and ``undefined`` (23) are assigned. *libcbor* represents all of them as ctrl items:
:func:`cbor_ctrl_value` returns the raw value, and :func:`cbor_is_bool`,
:func:`cbor_is_null`, and :func:`cbor_is_undef` identify the assigned ones.

The unassigned values 0 to 19 and 32 to 255 are well-formed and are decoded and
encoded as-is. Values 24 to 31 are reserved and cannot be represented in well-formed
CBOR; the decoders reject them, and :func:`cbor_build_ctrl` will produce an encoding
that other decoders will reject as well.

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

Reading data
~~~~~~~~~~~~~

.. doxygenfunction:: cbor_float_get_float2
.. doxygenfunction:: cbor_float_get_float4
.. doxygenfunction:: cbor_float_get_float8
.. doxygenfunction:: cbor_float_get_float
.. doxygenfunction:: cbor_ctrl_value
.. doxygenfunction:: cbor_get_bool

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
.. doxygenfunction:: cbor_set_bool
.. doxygenfunction:: cbor_set_float2
.. doxygenfunction:: cbor_set_float4
.. doxygenfunction:: cbor_set_float8


.. _api_type_7_floats_ctrls_half_floats:

Half floats
~~~~~~~~~~~~
CBOR supports two `bytes wide ("half-precision") <https://en.wikipedia.org/wiki/Half-precision_floating-point_format>`_
floats which are not supported by the C language. *libcbor* represents them using `float <https://en.cppreference.com/w/c/language/type>`_ values throughout the API. Encoding will be performed by :func:`cbor_encode_half`, which will handle any values that cannot be represented as a half-float.

NaN payloads
~~~~~~~~~~~~~~~~

The encoders preserve the IEEE 754 bit pattern of NaNs, i.e. the sign, the
quiet bit, and the payload (for half-precision floats, the top 10 bits of the
payload). Decoding an encoded NaN and encoding it again reproduces the
original bytes.

`Signaling NaNs <https://en.wikipedia.org/wiki/NaN#Signaling_NaN>`_ are
preserved on a best-effort basis only: some CPUs (notably x87, i.e. 32-bit x86)
set the quiet bit whenever a NaN passes through a floating point register,
which happens when the value is returned from :func:`cbor_float_get_float4`
and friends. The meaning of the quiet bit also differs between IEEE 754-2008
and legacy MIPS; *libcbor* does not translate it. See
https://github.com/PJK/libcbor/issues/215 and
https://github.com/PJK/libcbor/issues/336 for context.
