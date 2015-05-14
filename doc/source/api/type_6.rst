Type 6 – Semantic tags 
=============================

Tag are additional metadata that can be used to extend or specialize the meaning or interpretation of the other data items.

For example, one might tag an array of numbers to communicate that it should be interpreted as a vector.

Please consult the official `IANA repository of CBOR tags <https://www.iana.org/assignments/cbor-tags/cbor-tags.xhtml>`_ before inventing new ones.

.. doxygenfunction:: cbor_new_tag
.. doxygenfunction:: cbor_tag_item
.. doxygenfunction:: cbor_tag_value
.. doxygenfunction:: cbor_tag_set_item

