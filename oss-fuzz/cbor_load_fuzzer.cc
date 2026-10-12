#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "cbor.h"

static size_t allocated_mem = 0;
static constexpr size_t kMemoryLimit = 1 << 30;

// Each block keeps its size in a header in front of the returned pointer.
// A side table would allocate on insert, and a std::bad_alloc thrown from
// inside libcbor's allocator would terminate the process instead of
// returning NULL.
struct alignas(std::max_align_t) BlockHeader {
    size_t size;
    size_t magic;
};
static constexpr size_t kBlockMagic = 0x63626f72;

static BlockHeader *header_of(void *ptr) {
    BlockHeader *header = static_cast<BlockHeader *>(ptr) - 1;
    if (header->magic != kBlockMagic) {
        abort();
    }
    return header;
}

void *limited_malloc(size_t size) {
    if (size > kMemoryLimit - allocated_mem) {
        return nullptr;
    }
    if (size == 0) {
        return nullptr;
    }
    BlockHeader *header =
        static_cast<BlockHeader *>(malloc(sizeof(BlockHeader) + size));
    if (header == nullptr) {
        return nullptr;
    }
    header->size = size;
    header->magic = kBlockMagic;
    allocated_mem += size;
    return header + 1;
}

void limited_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    BlockHeader *header = header_of(ptr);
    allocated_mem -= header->size;
    header->magic = 0;
    free(header);
}

void *limited_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return limited_malloc(size);
    }
    BlockHeader *header = header_of(ptr);
    if (size == 0) {
        limited_free(ptr);
        return nullptr;
    }
    size_t old_size = header->size;
    if (size > kMemoryLimit - (allocated_mem - old_size)) {
        return nullptr;
    }
    BlockHeader *new_header = static_cast<BlockHeader *>(
        realloc(header, sizeof(BlockHeader) + size));
    if (new_header == nullptr) {
        return nullptr;
    }
    new_header->size = size;
    allocated_mem = allocated_mem - old_size + size;
    return new_header + 1;
}

struct State {
    FILE* fout;

    State() : fout(fopen("/dev/null", "w")) {
        cbor_set_allocs(limited_malloc, limited_realloc, limited_free);
    }
};

static State kState;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    cbor_load_result result;
    cbor_item_t *item = cbor_load(Data, Size, &result);
    if (result.error.code == CBOR_ERR_NONE) {
        cbor_describe(item, kState.fout);
        unsigned char *buffer;
        size_t buffer_size;
        cbor_serialize_alloc(item, &buffer, &buffer_size);
        limited_free(buffer);
        cbor_item_t *copied = cbor_copy(item);
        if (copied != NULL) {
            cbor_decref(&copied);
        }
        cbor_decref(&item);
    }
    return 0;
}
