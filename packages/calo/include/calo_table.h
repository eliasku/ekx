#ifndef CALO_TABLE_H
#define CALO_TABLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    // records count
    uint32_t count;
    // data array size
    uint32_t size;
    uint32_t* lengths;
    uint32_t* offsets;
    void* data;
} vla_table_t;

uint32_t add_string(vla_table_t* table, const char* str);

uint32_t add_data(vla_table_t* table, const void* data, uint32_t length);

const char* get_data(const vla_table_t* table, uint32_t index);

void free_table(vla_table_t* table);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
