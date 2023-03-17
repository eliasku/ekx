#include <calo_stream.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void read_span(calo_reader_t* r, void* dest, uint32_t size) {
    memcpy(dest, r->p, size);
    r->p += size;
}

uint8_t read_u8(calo_reader_t* r) {
    uint8_t val;
    read_span(r, &val, sizeof val);
    return val;
}

uint16_t read_u16(calo_reader_t* r) {
    uint16_t val;
    read_span(r, &val, sizeof val);
    return val;
}

uint32_t read_u32(calo_reader_t* r) {
    uint32_t val;
    read_span(r, &val, sizeof val);
    return val;
}

int32_t read_i32(calo_reader_t* r) {
    int32_t val;
    read_span(r, &val, sizeof val);
    return val;
}

float read_f32(calo_reader_t* r) {
    float val;
    read_span(r, &val, sizeof val);
    return val;
}

double read_f64(calo_reader_t* r) {
    double val;
    read_span(r, &val, sizeof val);
    return val;
}

const char* read_stream_string(calo_reader_t* r) {
    uint32_t handle = read_u32(r);
    return get_data(&r->strings, handle);
}

const void* read_stream_data(calo_reader_t* r) {
    uint32_t handle = read_u32(r);
    return get_data(&r->strings, handle);
}

const char* map_stream_string(calo_reader_t* r) {
    uint32_t size = read_u32(r);
    const char* val = (const char*) r->p;
    r->p += size;
    return val;
}

void fwrite_table(FILE* f, vla_table_t* t) {
    fwrite(&t->count, 1, sizeof(uint32_t), f);
    fwrite(&t->size, 1, sizeof(uint32_t), f);
    fwrite(t->offsets, 1, sizeof(uint32_t) * t->count, f);
    fwrite(t->lengths, 1, sizeof(uint32_t) * t->count, f);
    fwrite(t->data, 1, t->size, f);
}

void fwrite_calo(FILE* f, calo_writer_t* w) {
    fwrite_table(f, &w->strings);
    fwrite(w->data, 1, w->p - (uint8_t*) w->data, f);
}

void read_calo(calo_reader_t* r) {
    vla_table_t* t = &r->strings;
    t->count = read_u32(r);
    t->size = read_u32(r);
    // TODO: can allocate single BLOB for whole structure
    t->offsets = (uint32_t*) malloc(sizeof(uint32_t) * t->count);
    t->lengths = (uint32_t*) malloc(sizeof(uint32_t) * t->count);
    t->data = malloc(t->size);
    read_span(r, t->offsets, sizeof(uint32_t) * t->count);
    read_span(r, t->lengths, sizeof(uint32_t) * t->count);
    read_span(r, t->data, t->size);
}

void map_table(calo_reader_t* r, vla_table_t* table) {
    const uint32_t count = read_u32(r);
    const uint32_t data_size = read_u32(r);
    const uint32_t desc_length = sizeof(uint32_t) * count;
    table->count = count;
    table->size = data_size;
    table->offsets = (uint32_t*) r->p;
    r->p += desc_length;
    table->lengths = (uint32_t*) r->p;
    r->p += desc_length;
    table->data = (void*) r->p;
    r->p += data_size;
}

/**** WRITER API ****/

void free_writer(calo_writer_t* w) {
    free(w->data);
    w->data = NULL;
    w->p = NULL;
    free_table(&w->strings);
}

calo_writer_t new_writer(uint32_t reserved) {
    calo_writer_t w = {0};
    w.data = malloc(reserved);
    w.p = w.data;
    w.cap = reserved;
    return w;
}

void ensure_writer_space(calo_writer_t* w, uint32_t bytes_to_write) {
    uint32_t pos = w->p - (uint8_t*) w->data;
    uint32_t new_len = pos + bytes_to_write;
    uint32_t cap = w->cap;
    if (new_len > cap) {
        while (new_len > cap) {
            cap <<= 1;
        }
        w->data = realloc(w->data, cap);
        w->cap = cap;
        w->p = (uint8_t*) w->data + pos;
    }
}

void write_span(calo_writer_t* w, const void* data, uint32_t size) {
    ensure_writer_space(w, size);
    memcpy(w->p, data, size);
    w->p += size;
}

void write_u8(calo_writer_t* w, uint8_t v) {
    write_span(w, &v, sizeof v);
}

void write_u16(calo_writer_t* w, uint16_t v) {
    write_span(w, &v, sizeof v);
}

void write_u32(calo_writer_t* w, uint32_t v) {
    write_span(w, &v, sizeof v);
}

void write_i32(calo_writer_t* w, int32_t v) {
    write_span(w, &v, sizeof v);
}

void write_f32(calo_writer_t* w, float v) {
    write_span(w, &v, sizeof v);
}

void write_f64(calo_writer_t* w, double v) {
    write_span(w, &v, sizeof v);
}

void write_stream_data(calo_writer_t* w, const void* data, uint32_t size) {
    uint32_t handle = add_data(&w->strings, data, size);
    write_u32(w, handle);
}

void write_stream_string(calo_writer_t* w, const char* s) {
    uint32_t handle = add_string(&w->strings, s);
    write_u32(w, handle);
}
