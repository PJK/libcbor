API
=======

The data API is centered around :type:`cbor_item_t`, a generic handle for any CBOR item. There are functions to

 - create items,
 - set items' data,
 - parse serialized data into items,
 - manage, move, and links item together.

The single most important thing to keep in mind is: :type:`cbor_item_t` **is an opaque type and should only be manipulated using the appropriate functions!** Think of it as an object.

The *libcbor* API closely follows the semantics outlined by `CBOR standard <http://tools.ietf.org/html/rfc7049>`_. This part of the documentation provides a short overview of the CBOR constructs, as well as a general introduction to the *libcbor* API. Remaining reference can be found in the following files structured by data types

.. toctree::

   api/type_0_1
   api/type_2
   api/type_3
   api/type_4
   api/type_5
   api/type_6
   api/type_7

Every :type:`cbor_item_t` has a :type:`cbor_type` associated with it - these constants correspond to the types specified by the `CBOR standard <http://tools.ietf.org/html/rfc7049>`_:

.. type:: enum cbor_type

   which consists of

   +---------------------------+
   | ``CBOR_TYPE_UINT``        |
   +---------------------------+
   | ``CBOR_TYPE_NEGINT``      |
   +---------------------------+
   | ``CBOR_TYPE_BYTESTRING``  |
   +---------------------------+
   | ``CBOR_TYPE_STRING``      |
   +---------------------------+
   | ``CBOR_TYPE_ARRAY``       |
   +---------------------------+
   | ``CBOR_TYPE_MAP``         |
   +---------------------------+
   | ``CBOR_TYPE_TAG``         |
   +---------------------------+
   | ``CBOR_TYPE_FLOAT_CTRL``  |
   +---------------------------+

A quick CBOR primer
--------------------------

TODO, please refer to :rfc:`7049`

The API
-----------------

The API is designed to allow both very tight control & flexibility and general convenience with sane defaults. [#]_ For example, client with very specific requirements (constrained environment, custom application protocol built on top of CBOR, etc.) may choose to take full control (and responsibility) of memory and data structures management by interacting directly with the decoder. Other clients might want to take control of specific aspects (streamed collections, hash maps storage), but leave other responsibilities to *libcbor*. More general clients might prefer to be abstracted away from all aforementioned details and only be presented complete data structures.

  .. [#] http://softwareengineering.vazexqi.com/files/pattern.html

*libcbor* provides
 - stateless encoders and decoders
 - encoding and decoding *drivers*, routines that coordinate encoding and decoding of complex structures
 - data structures to represent and transform CBOR structures
 - routines for building and manipulating these structures
 - utilities for inspection and debugging

Decoding
~~~~~~~~~~
::

    ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    │                                                                                              │
    │                                      Client application                                      │
    │                                                                                              │
    │                                                 ┌────────────────────────────────────────────┘
    │                                                 │                     ↕
    │                                                 │ ┌──────────────────────────────────────────┐
    │                                                 │ │                                          │
    │                                                 │ │          Manipulation routines           │
    │                                                 │ │                                          │
    │           ┌─────────────────────────────────────┘ └──────────────────────────────────────────┘
    │           │     ↑    ↑                  ↑                              ↑
    │           │     │    │    ┌─────────────╫──────────┬───────────────────┴─┐
    │           │     │   CDS   │             ║          │                     │
    │           │     │    │   PDS            ║         PDS                   PDS
    │           │     ↓    ↓    ↓             ↓          ↓                     ↓
    │           │ ┌─────────────────┐   ┌────────────────────┐   ┌────────────────────────────┐
    │           │ │                 │   │                    │   │                            │
    │           │ │  Custom driver  │ ↔ │  Streaming driver  │ ↔ │       Default driver       │ ↔ CD
    │           │ │                 │   │                    │   │                            │
    └───────────┘ └─────────────────┘   └────────────────────┘   └────────────────────────────┘
          ↕                ↕                        ↕                           ↕
    ┌──────────────────────────────────────────────────────────────────────────────────────────────┐
    │                                                                                              │
    │                            Stateless event─driven decoder                                    │
    │                                                                                              │
    └──────────────────────────────────────────────────────────────────────────────────────────────┘

                  (PSD = Provided Data Structures, CDS = Custom Data Structures)



Encoding
~~~~~~~~~~
TODO

Memory management
^^^^^^^^^^^^^^^^^^^

Due to the nature of its domain *libcbor* will need to work with heap memory. The stateless decoder and encoder don't allocate any memory.

If you have specific requirements, you should consider rolling your own driver for the stateless API.

Using custom allocator
^^^^^^^^^^^^^^^^^^^^^^^^

*libcbor* gives you with the ability to provide your own implementations of ``malloc``, ``realloc``, and ``free``. This can be useful if you are using a custom allocator throughout your application, or if you want to implement custom policies (e.g. tighter restrictions on the amount of allocated memory).

In order to use this feature, *libcbor* has to be compiled with the :doc:`appropriate flags <getting_started>`. You can verify the configuration using the ``CBOR_CUSTOM_ALLOC`` macro. A simple usage might be as follows:

.. code-block:: c

    #ifdef CBOR_CUSTOM_ALLOC
        cbor_set_allocs(malloc, realloc, free);
    #else
       #error "libcbor built with support for custom allocation is required"
    #endif

.. function:: void cbor_set_allocs(_cbor_malloc_t custom_malloc, _cbor_realloc_t custom_realloc, _cbor_free_t custom_free)

    Assigns the memory management functions globally.

    .. warning:: This function modifies the global state and should therefore be used accordingly. Changing the memory handlers while allocated items exist will result in a ``free``/``malloc`` mismatch. This function is not thread safe with respect to both itself nor all the other *libcbor* functions that work with the heap.


Reference counting
^^^^^^^^^^^^^^^^^^^^^

As CBOR items may require complex cleanups at the end of their lifetime, there is a reference counting mechanism in place. This also enables very simple GC when integrating *libcbor* into managed environment. Every item starts its life (by either explicit creation, or as a result of parsing) with reference count set to 1. When the refcount reaches zero, it will be destroyed.

Items containing nested items will be destroyed recursively - refcount of every nested item will be decreased by one.

The destruction is synchronous and might possibly render any pointers to items with refcount zero invalid immediately after calling the ``decref``.


.. function:: void cbor_incref(cbor_item_t * item)

	Increment the reference counter.

.. function:: void cbor_decref(cbor_item_t ** item)

	Decrement the reference counter. Deallocate the item if it reaches zero and recursively decref all dependent items.

.. _item-types:

Loading items
^^^^^^^^^^^^^^^^^^^^^

.. function:: cbor_item_t * cbor_load(const unsigned char * source, size_t source_size, size_t flags, struct cbor_load_result * result)

    Reads up to ``source_size`` bytes from memory (starting with ``source``) and returns a pointer to the loaded item. Success or failure (as well as the number of bytes actually read) is indicated through ``result``. On failure, the returned pointer is ``NULL``.


.. type:: struct cbor_load_result
	
	Describes result of loading an item. Contains the following members:

	.. c:member:: struct cbor_error error
		
		Detailed description of the problem, including location.

	.. c:member:: size_t read
    
		Number of bytes read. 

.. type:: struct cbor_error
	
	Describes an error that occurred during an attempt to load an item. Contains the following members:

	.. c:member:: size_t position
		
		Position where the error condition was identified, relative to the starting byte. Please note that this might not be the position of the true cause.
	
	.. c:member:: cbor_error_code code

		Indication of the kind of the problem.

.. type:: enum cbor_error_code

	TODO


Types of items
^^^^^^^^^^^^^^^^^^^^^

A :type:`cbor_item_t` can be probed for its type using these functions

.. function:: bool cbor_isa_uint(const cbor_item_t * item)
.. function:: bool cbor_isa_negint(const cbor_item_t * item)
.. function:: bool cbor_isa_bytestring(const cbor_item_t * item)
.. function:: bool cbor_isa_string(const cbor_item_t * item)
.. function:: bool cbor_isa_array(const cbor_item_t * item)
.. function:: bool cbor_isa_map(const cbor_item_t * item)
.. function:: bool cbor_isa_tag(const cbor_item_t * item)
.. function:: bool cbor_isa_float_ctrl(const cbor_item_t * item)
