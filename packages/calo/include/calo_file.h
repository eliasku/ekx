#ifndef CALO_FILE_H
#define CALO_FILE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void* read_file(const char* filepath, uint32_t* out_size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CALO_FILE_H
