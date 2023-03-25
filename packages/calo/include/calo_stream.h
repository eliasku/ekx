#ifndef CALO_STREAM_H
#define CALO_STREAM_H

#include <stdint.h>
#include <stdio.h>
#include "calo_table.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
    const uint8_t* p;
    vla_table_t strings;
} calo_reader_t;

void read_calo(calo_reader_t* r);
void read_span(calo_reader_t* r, void* dest, uint32_t size);
uint8_t read_u8(calo_reader_t* r);
uint16_t read_u16(calo_reader_t* r);
uint32_t read_u32(calo_reader_t* r);
int32_t read_i32(calo_reader_t* r);
float read_f32(calo_reader_t* r);
double read_f64(calo_reader_t* r);
const char* read_stream_string(calo_reader_t* r);
const void* read_stream_data(calo_reader_t* r);
const char* map_stream_string(calo_reader_t* r);

/**
 *
 * @param r reader
 * @param table table structure
 */
void map_table(calo_reader_t* r, vla_table_t* table);

void fwrite_table(FILE* stream, vla_table_t* table);

typedef struct {
    uint8_t* p;
    uint8_t* data;
    uint32_t cap;
    vla_table_t strings;
} calo_writer_t;

calo_writer_t new_writer(uint32_t reserved);
void free_writer(calo_writer_t* w);
void ensure_writer_space(calo_writer_t* w, uint32_t bytes_to_write);
void fwrite_calo(FILE* f, calo_writer_t* w);
void write_span(calo_writer_t* w, const void* data, uint32_t size);
void write_u8(calo_writer_t* w, uint8_t v);
void write_u16(calo_writer_t* w, uint16_t v);
void write_u32(calo_writer_t* w, uint32_t v);
void write_i32(calo_writer_t* w, int32_t v);
void write_f32(calo_writer_t* w, float v);
void write_f64(calo_writer_t* w, double v);
void write_stream_data(calo_writer_t* w, const void* data, uint32_t size);
void write_stream_string(calo_writer_t* w, const char* s);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif

