#include <stdio.h>
#include <stdlib.h>
#include <calo_file.h>

void* read_file(const char* filepath, uint32_t* out_size) {
    FILE* f = fopen(filepath, "rb");
    fseek(f, 0, SEEK_END);
    const uint32_t filesize = (uint32_t)ftell(f);
    if (out_size) {
        *out_size = filesize;
    }
    fseek(f, 0, SEEK_SET);
    void* mem = malloc(filesize);
    fread(mem, 1, filesize, f);
    fclose(f);
    return mem;
}
