#include <calo_table.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define resize_alloc(Array, Count)  do{ (Array)[0] = (__typeof__((Array)[0])) realloc((Array)[0], (sizeof (Array)[0][0]) * Count); }while(0)

uint32_t find_data_(const vla_table_t* table, const void* data, uint32_t size) {
    for (uint32_t i = 0; i < table->count; ++i) {
        const void* record_data = get_data(table, i);
        const uint32_t record_size = table->lengths[i];
        if (size == record_size && memcmp(data, record_data, size) == 0) {
            return i;
        }
    }
    return ~0;
}

uint32_t add_data(vla_table_t* table, const void* data, uint32_t length) {
    uint32_t found = find_data_(table, data, length);
    if (found != ~0) {
        return found;
    }

    const uint32_t offset = table->size;
    const uint32_t index = table->count++;
    if (!data && length) {
        // length should be 0 for NULL data
        abort();
    }
    resize_alloc(&table->lengths, table->count);
    resize_alloc(&table->offsets, table->count);
//    resize_alloc(&table->hash_index, table->count);
    table->lengths[index] = length;
    table->offsets[index] = offset;
//    table->hash_index[index] = (data_hash_index_t) {
//            .index = index,
//            .hash = 0
//    };

    if (length) {
        table->size += length;
        resize_alloc((char**) &table->data, table->size);
        void* end = (char*) table->data + offset;
        memcpy(end, data, length);
    }

    return index;
}

uint32_t add_string(vla_table_t* table, const char* str) {
    const uint32_t max_length = 1024;
    const uint32_t data_length = strnlen(str, max_length);
    // no null-terminator is invalid
    if (data_length == max_length) {
        // exceeded string max length
        abort();
    }

    return add_data(table, str, data_length + 1);
}

const char* get_data(const vla_table_t* table, uint32_t index) {
    return (const char*) table->data + table->offsets[index];
}

void free_table(vla_table_t* table) {
    if (table) {
        free(table->offsets);
        free(table->lengths);
        free(table->data);
    }
    *table = (vla_table_t) {};
}

#undef resize_alloc
