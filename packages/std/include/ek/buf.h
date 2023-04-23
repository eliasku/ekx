#ifndef EK_BUF_H
#define EK_BUF_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_buf_header_t {
    uint32_t pad0;
    uint32_t pad1;
    uint32_t capacity;
    uint32_t length;
} ek_buf_header_t;

ek_buf_header_t* ek_buf_header(const void* ptr);

uint32_t arr_capacity(const void* ptr);

uint32_t arr_size(const void* ptr);

void arr_clear(const void* ptr);

bool arr_full(const void* ptr);

bool arr_empty(const void* ptr);

void arr_reset_(void** ptr);

#define arr_reinit(arr, size) ek_buf_set_size((void**)&(arr), sizeof (arr)[0], size, size)

void ek_buf_set_capacity(void** ptr, uint32_t newCapacity, uint32_t elementSize);

void ek_buf_set_size(void** buf, uint32_t elementSize, uint32_t len, uint32_t cap);

void* ek_buf_add_(void* ptr, uint32_t elementSize);

void* ek_buf_remove_(void* ptr, uint32_t i, uint32_t elementSize);

void* ek_buf_pop_(void* ptr, uint32_t elementSize);

// pod array impl
void arr_init_from(void** arr, uint32_t element_size, const void* src, uint32_t len);

void arr_resize_(void** p_arr, uint32_t element_size, uint32_t new_len);

void arr_grow(void** p_arr, uint32_t capacity, uint32_t element_size);

void arr_maybe_grow(void** p_arr, uint32_t element_size);

void* arr_push_mem(void** p_arr, uint32_t element_size, const void* src);

void arr_assign_(void** p_arr, uint32_t element_size, const void* src_arr);

#define arr_assign(arr_dest, arr_src) arr_assign_((void**)&(arr_dest), sizeof (arr_dest)[0], (void*)src_arr)

void arr_swap_remove_(void* arr, uint32_t element_size, uint32_t at);

void* arr_search(void* arr, uint32_t element_size, const void* el);

void arr_pop(void* arr);

void arr_erase_(void* arr, const void* it, uint32_t element_size, uint32_t count);
#define arr_erase(arr, ptr) arr_erase_(arr, it, sizeof *(arr), 1)
#define arr_erase_at(arr, index) arr_erase_(arr, (arr) + (index), sizeof *(arr), 1)

/**
 * internal function ensure we have space for one more element in array
 * @param p_arr - pointer to buffer
 * @param element_size - array's element size
 * @return pointer to slot we can assign new element
 */
void* arr_add_(void** p_arr, uint32_t element_size);

#define arr_reset(arr) arr_reset_((void**)&(arr))
#define arr_push_(p_arr, T, el) (*((T*)arr_add_((void**)(p_arr), sizeof(T))) = (el))
#define arr_push(arr, el) ((__typeof__ (arr))arr_add_((void**)&(arr), sizeof (arr)[0]))[0] = (el)
#define arr_resize(arr, cnt) (arr_resize_((void**)&(arr), sizeof (arr)[0], (cnt)))
#define arr_begin(arr) (arr)
#define arr_end(arr) ((arr) + arr_size(arr))
#define arr_back(arr) ((arr) ? (arr_end(arr) - 1) : 0)

#define arr_for_block_(arr, Type, It, End) for(Type * It = (arr), * End = (arr) + arr_size(arr); It != End; ++It)
#define arr_for(Var, InArray) arr_for_block_((InArray), __typeof__(*(InArray)), Var, Var ## _end)

void* _check_ptr_alignment(void* ptr, uint32_t width);

#define cast_ptr_aligned(T, ptr) ((T*)_check_ptr_alignment(ptr, 4))

// fixed array utilities

#define COUNT_OF(FixedArray) (sizeof(FixedArray) / sizeof(*(FixedArray)))
#define DEFINE_ARRAY_FIND(T) static (T)* find_##T((T)* array, uint32_t n, T element) { \
    for (uint32_t i = 0; i < n; ++i) { \
        if (array[i] == element) return array + i; \
    } \
    return NULL; \
}


const char* str_get(void* buf);
void str_copy(void** p_dest, const void* src);
void str_init_c_str(void** p_dest, const char* c_str);

#ifdef __cplusplus
}
#endif

#endif // EK_BUF_H
