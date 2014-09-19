RFC conformance
=========================

*libcbor* is, generally speaking, very faithful implementation of `RFC 7049 <http://tools.ietf.org/html/rfc7049>`_. There are, however, some limitations imposed by technical constraints.

Bytestring length
-------------------
There is no explicit limitation of indefinite length bytestrings. [#]_ *libcbor* will not handle bytestrings with more chunks than the maximum value of :type:`size_t`. On any sane platform, such string would not fit in the memory anyway. It is, however, possible to process arbitrarily long strings and bytestrings using the
TODO chunked mode.

.. [#] http://tools.ietf.org/html/rfc7049#section-2.2.2

Map keys
--------
The standard allows :doc:`map's <api/type_5>` keys to be any valid data items, not only strings. [#]_ This results in *libcbor* not being able to verify the uniqueness constraint [#]_ in streaming mode.
TODO can we verify it at all?

.. [#]  http://tools.ietf.org/html/rfc7049#section-3.7
.. [#]  http://tools.ietf.org/html/rfc7049#section-3.10


