set(SOURCES cbor.c cbor_internal.c cbor/internal/stack.c cbor/internal/unicode.c cbor/encoding.c cbor/serialization.c cbor/arrays.c cbor/common.c cbor/floats_ctrls.c cbor/bytestrings.c cbor/strings.c cbor/maps.c cbor/tags.c cbor/ints.c)

set(CMAKE_SKIP_BUILD_RPATH FALSE)


if(CUSTOM_ALLOC)
    LIST(APPEND SOURCES allocators.c)
endif(CUSTOM_ALLOC)

if (NOT DEFINED CMAKE_MACOSX_RPATH)
	set(CMAKE_MACOSX_RPATH 0)
endif()

add_library(cbor ${SOURCES})
add_library(cbor_shared SHARED ${SOURCES})

set_target_properties(cbor_shared PROPERTIES OUTPUT_NAME cbor)

#http://www.cmake.org/Wiki/CMake:Install_Commands

install(TARGETS cbor cbor_shared
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin)

install(DIRECTORY cbor DESTINATION include
                FILES_MATCHING PATTERN "*.h")

install(FILES cbor.h DESTINATION include)
