Decoding
=============================
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
