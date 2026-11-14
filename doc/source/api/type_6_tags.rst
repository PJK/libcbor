Type 6 – Semantic tags 
=============================

Tags are additional metadata that can be used to extend or specialize the meaning or interpretation of the other data items.

For example, one might tag an array of numbers to communicate that it should be interpreted as a vector.

Please note that libcbor does *not* understand the semantics of tags and will
process all `well-formed <https://datatracker.ietf.org/doc/html/rfc8949#name-terminology>`_
tags regardless of whether they are valid with respect to the data they are
applied to. For example, tag 0 (``CBOR_TAG_DATETIME_STRING``, standard date/time
string) applied to a map will be decoded without error, even though
`RFC 8949 Section 5.3.2 <https://www.rfc-editor.org/rfc/rfc8949#section-5.3.2>`_
considers it invalid. Applications that rely on a tag's meaning should check
the type of :func:`cbor_tag_item` themselves. See :ref:`decoder-validity` for
the general policy.

==================================  ======================================================
Corresponding :type:`cbor_type`     ``CBOR_TYPE_TAG``
Number of allocations               One plus any manipulations with the data
Storage requirements                ``sizeof(cbor_item_t) + the tagged item``
==================================  ======================================================

Standard tag numbers
~~~~~~~~~~~~~~~~~~~~

*libcbor* defines ``uint64_t`` macro constants in ``cbor/tags.h`` for the 15
core tag numbers in
`RFC 8949 Section 3.4, Table 5 <https://www.rfc-editor.org/rfc/rfc8949#section-3.4>`_
(note that tag 35 from RFC 7049 was omitted from Table 5 in RFC 8949 and is not
included). Tags registered in the broader
`IANA repository of CBOR tags <https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml>`_
are intentionally left to application code. All tag functions accept and return
arbitrary ``uint64_t`` values, not only these constants.

.. doxygendefine:: CBOR_TAG_DATETIME_STRING
.. doxygendefine:: CBOR_TAG_DATETIME_EPOCH
.. doxygendefine:: CBOR_TAG_UNSIGNED_BIGNUM
.. doxygendefine:: CBOR_TAG_NEGATIVE_BIGNUM
.. doxygendefine:: CBOR_TAG_DECIMAL_FRACTION
.. doxygendefine:: CBOR_TAG_BIGFLOAT
.. doxygendefine:: CBOR_TAG_EXPECTED_BASE64URL
.. doxygendefine:: CBOR_TAG_EXPECTED_BASE64
.. doxygendefine:: CBOR_TAG_EXPECTED_BASE16
.. doxygendefine:: CBOR_TAG_ENCODED_CBOR
.. doxygendefine:: CBOR_TAG_URI
.. doxygendefine:: CBOR_TAG_BASE64URL
.. doxygendefine:: CBOR_TAG_BASE64
.. doxygendefine:: CBOR_TAG_MIME_MESSAGE
.. doxygendefine:: CBOR_TAG_SELF_DESCRIBED_CBOR

Tag manipulation
~~~~~~~~~~~~~~~~

.. doxygenfunction:: cbor_new_tag
.. doxygenfunction:: cbor_build_tag
.. doxygenfunction:: cbor_tag_item
.. doxygenfunction:: cbor_tag_value
.. doxygenfunction:: cbor_tag_set_item

