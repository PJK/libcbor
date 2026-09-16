Type 6 – Semantic tags 
=============================

Tags are additional metadata that can be used to extend or specialize the meaning or interpretation of the other data items.

For example, one might tag an array of numbers to communicate that it should be interpreted as a vector.

Please consult the official `IANA repository of CBOR tags <https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml>`_ for known registered values.

Please note that libcbor does *not* understand the semantics of tags and will
process all `well-formed <https://datatracker.ietf.org/doc/html/rfc8949#name-terminology>`_
tags regardless of whether they are valid with respect to the data they are
applied to. For example, tag 0 (standard date/time string) applied to a map
will be decoded without error, even though
`RFC 8949 Section 5.3.2 <https://www.rfc-editor.org/rfc/rfc8949#section-5.3.2>`_
considers it invalid. Applications that rely on a tag's meaning should check
the type of :func:`cbor_tag_item` themselves. See :ref:`decoder-validity` for
the general policy.

==================================  ======================================================
Corresponding :type:`cbor_type`     ``CBOR_TYPE_TAG``
Number of allocations               One plus any manipulations with the data
Storage requirements                ``sizeof(cbor_item_t) + the tagged item``
==================================  ======================================================

.. doxygenfunction:: cbor_new_tag
.. doxygenfunction:: cbor_build_tag
.. doxygenfunction:: cbor_tag_item
.. doxygenfunction:: cbor_tag_value
.. doxygenfunction:: cbor_tag_set_item

