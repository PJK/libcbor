API
=======

.. toctree::

   api/item_types
   api/item_reference_counting
   api/type_0_1
   api/type_2
   api/type_3
   api/type_4
   api/type_5
   api/type_6
   api/type_7

The data API is centered around :type:`cbor_item_t`, a generic handle for any CBOR item. There are functions to

 - create items,
 - set items' data,
 - parse serialized data into items,
 - manage, move, and links item together.

The single most important thing to keep in mind is: :type:`cbor_item_t` **is an opaque type and should only be manipulated using the appropriate functions!** Think of it as an object.

The *libcbor* API closely follows the semantics outlined by `CBOR standard <http://tools.ietf.org/html/rfc7049>`_. This part of the documentation provides a short overview of the CBOR constructs, as well as a general introduction to the *libcbor* API. Remaining reference can be found in the following files structured by data types



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
