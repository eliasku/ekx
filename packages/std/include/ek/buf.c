#include <ek/buf.h>
#include <ek/assert.h>
#include "log.h"

void* _check_ptr_alignment(void* ptr, uint32_t width) {
    EK_ASSERT(((uintptr_t) ptr % width) == 0);
    return ptr;
}

ek_buf_header_t* ek_buf_header(const void* ptr) {
    EK_ASSERT_R2(ptr != 0);
    return ((ek_buf_header_t*) ptr) - 1;
}

uint32_t arr_capacity(const void* ptr) {
    // TODO: add test for empty array
    return ptr ? ek_buf_header(ptr)->capacity : 0;
}

uint32_t arr_size(const void* ptr) {
    // TODO: add test for empty array
    return ptr ? ek_buf_header(ptr)->length : 0;
}

void arr_clear(const void* ptr) {
    if (ptr) {
        ek_buf_header(ptr)->length = 0;
    }
}

bool arr_full(const void* ptr) {
    // TODO: add test for empty array
    if (ptr) {
        const ek_buf_header_t* hdr = ek_buf_header(ptr);
        return hdr->capacity == hdr->length;
    }
    return true;
}

bool arr_empty(const void* ptr) {
    return !ptr || !arr_size(ptr);
}

void arr_reset(void** ptr) {
    if (*ptr) {
        // destroy buffer content
        ek_buf_header_t* hdr = ek_buf_header(*ptr);
        free(hdr);
        *ptr = 0;
    }
}

void ek_buf_set_capacity(void** ptr, uint32_t newCapacity, uint32_t elementSize) {
    // delete case
    if (newCapacity == 0) {
        arr_reset(ptr);
        return;
    }
    ek_buf_header_t* prevHeader = *ptr ? ek_buf_header(*ptr) : 0;
    if (prevHeader && prevHeader->capacity == newCapacity) {
        return;
    }

    // reallocate buffer content with different capacity
    const uint32_t size = sizeof(ek_buf_header_t) + newCapacity * elementSize;
    EK_PROFILE_ALLOC("buf realloc", size);
    ek_buf_header_t* hdr = (ek_buf_header_t*) realloc(prevHeader, size);
    hdr->capacity = newCapacity;
    if (!prevHeader) {
        hdr->length = 0;
    }
    *ptr = (void*) (hdr + 1);
}

void ek_buf_set_size(void** buf, uint32_t element_size, uint32_t len, uint32_t cap) {
    if (arr_capacity(*buf) < cap) {
        ek_buf_set_capacity(buf, cap, element_size);
    }
    if (*buf) {
        ek_buf_header(*buf)->length = len;
    }
}

void* ek_buf_add_(void* ptr, uint32_t elementSize) {
    EK_ASSERT_R2(ptr != 0);
    return (char*) ptr + elementSize * (ek_buf_header(ptr)->length++);
}

void* ek_buf_remove_(void* ptr, uint32_t i, uint32_t elementSize) {
    EK_ASSERT_R2(ptr != 0);
    ek_buf_header_t* hdr = ek_buf_header(ptr);
    EK_ASSERT_R2(i < hdr->length);
    --hdr->length;
    return (char*) ptr + elementSize * i;
}

void* ek_buf_pop_(void* ptr, uint32_t elementSize) {
    EK_ASSERT_R2(ptr != 0);
    ek_buf_header_t* hdr = ek_buf_header(ptr);
    EK_ASSERT_R2(hdr->length != 0);
    return (char*) ptr + elementSize * (--hdr->length);
}

// pod array

/**
 * ensure buffer size and copy `len` elements from `src` memory (count * el_size)
 * @param arr array
 * @param element_size single element size
 * @param src data source
 * @param len data source byte length
 */
void arr_init_from(void** arr, uint32_t element_size, const void* src, uint32_t len) {
    ek_buf_set_size(arr, element_size, len, len);
    if (*arr && len) {
        memcpy(*arr, src, len * element_size);
    }
}

void arr_resize_(void** p_arr, uint32_t element_size, uint32_t new_len) {
    const uint32_t cap = arr_capacity(*p_arr);
    if (new_len > cap) {
        arr_grow(p_arr, new_len, element_size);
    }
    if (*p_arr) {
        ek_buf_header(*p_arr)->length = new_len;
    }
}

void arr_grow(void** arr, uint32_t capacity, uint32_t element_size) {
    EK_ASSERT_R2(capacity != 0);
    const uint32_t sz = arr_size(*arr);

    void* new_buffer = 0;
    ek_buf_set_size(&new_buffer, element_size, sz, capacity);
    memcpy(new_buffer, *arr, sz * element_size);

    arr_reset(arr);
    *arr = new_buffer;
}

void arr_maybe_grow(void** arr, uint32_t element_size) {
    if (arr_full(*arr)) {
        arr_grow(arr, *arr ? (arr_capacity(*arr) << 1) : 1, element_size);
    }
}

void* arr_push_mem(void** p_arr, uint32_t element_size, const void* src) {
    arr_maybe_grow(p_arr, element_size);
    void* slot = ek_buf_add_(*p_arr, element_size);
    memcpy(slot, src, element_size);
    return slot;
}

void arr_assign_(void** p_arr, uint32_t element_size, const void* src_arr) {
    EK_ASSERT(p_arr);
    EK_ASSERT(*p_arr != src_arr);
    const uint32_t other_size = arr_size(src_arr);
    if (other_size) {
        if (arr_capacity(*p_arr) < other_size) {
            // grow buffer
            ek_buf_set_capacity(p_arr, arr_capacity(src_arr), element_size);
        }
        if (*p_arr) {
            memcpy(*p_arr, src_arr, other_size * element_size);
        }
    }
    if (*p_arr) {
        ek_buf_header(*p_arr)->length = other_size;
    }
}

void arr_swap_remove_(void* arr, uint32_t element_size, uint32_t at) {
    EK_ASSERT(arr);
    ek_buf_header_t* hdr = ek_buf_header(arr);
    EK_ASSERT(at < hdr->length);
    // [A, A, A, X, B, B]
    // [A, A, A, B, B]
    --hdr->length;
    if (at != hdr->length) {
        void* p = (char*) arr + at * element_size;
        memcpy(p, (char*) arr + hdr->length * element_size, element_size);
    }
}

void* arr_search(void* arr, uint32_t element_size, const void* el) {
    EK_ASSERT(el);
    EK_ASSERT(element_size);
    if (arr) {
        const ek_buf_header_t* hdr = ek_buf_header(arr);
        const uint32_t len = hdr->length;
        char* ptr = (char*) arr;
        for (uint32_t i = 0; i < len; ++i) {
            if (memcmp(ptr, el, element_size) == 0) {
                return ptr;
            }
        }
    }
    return NULL;
}

void arr_pop(void* arr) {
    EK_ASSERT_R2(arr);
    ek_buf_header_t* hdr = ek_buf_header(arr);
    EK_ASSERT_R2(hdr->length);
    --hdr->length;
}

void arr_erase_(void* arr, const void* it, uint32_t element_size, uint32_t count) {
    if (arr) {
        ek_buf_header_t* hdr = ek_buf_header(arr);
        const char* end = (char*) arr + hdr->length * element_size;
        const char* next = (char*) it + element_size * count;
        EK_ASSERT(next <= end);
        if (next < end) {
            memmove((void*) it, next, end - next);
        }
        hdr->length -= count;
    }
}

void* arr_add_(void** p_arr, uint32_t element_size) {
    arr_maybe_grow(p_arr, element_size);
    return ek_buf_add_(*p_arr, element_size);
}

const char* str_get(void* buf) {
    return buf ? buf : "";
}

void str_copy(void** p_dest, const void* src) {
    arr_assign_(p_dest, 1, src);
}
