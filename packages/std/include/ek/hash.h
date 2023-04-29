#ifndef EK_HASH_H
#define EK_HASH_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_MURMUR2_64_DEFAULT_SEED 0x01000193

// MurMurHash2 (64-bit, aligned)
uint64_t hash_murmur2_64(const void* key, uint32_t len, uint64_t seed);
uint32_t hash_murmur3_32(const void* key, uint32_t len, uint32_t seed);

// inline hash calculation
// refs: https://github.com/haipome/fnv
// FNV-1a hash

#define HASH_FNV32_INIT (0x811C9DC5u)
#define HASH_FNV64_INIT (0xCBF29CE484222325ull)

uint64_t hash_fnv64(const char* str, uint64_t hash);
uint32_t hash_fnv32(const char* str, uint32_t hash);

// if you modify seed value, you should also modify it in `ekx` build tool project
#define H_SEED HASH_FNV32_INIT
typedef uint32_t string_hash_t;
const char* hsp_get(string_hash_t hv);

// https://stackoverflow.com/questions/34333936/how-to-force-gcc-compiler-to-calculate-constants-at-compile-time-with-os
#define HSP_LB__(s) { h ^= (uint32_t) s[i++]; h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24); }
#define HSP_C_4__(s, o) for (; i < ((o + 4) < sizeof(s) - 1 ? (o + 4) : sizeof(s) - 1); ) HSP_LB__(s)

#define HSP_C_16__(s, o)           \
  HSP_C_4__(s, o);                 \
  HSP_C_4__(s, o + 4);             \
  HSP_C_4__(s, o + 8);             \
  HSP_C_4__(s, o + 12)

#define HSP_C_64__(s, o)           \
  HSP_C_16__(s, o);                \
  HSP_C_16__(s, o + 16);           \
  HSP_C_16__(s, o + 32);           \
  HSP_C_16__(s, o + 48)

#define HSP_C_256__(s, o)          \
  HSP_C_64__(s, o);                \
  HSP_C_64__(s, o + 64);           \
  HSP_C_64__(s, o + 128);          \
  HSP_C_64__(s, o + 192)

#define HSP_C_STR(s, n) ({ \
      uint32_t i = 0;      \
      uint32_t h = H_SEED; \
      if (n - 1 < 256) {   \
        HSP_C_256__(s, 0); \
      } else { \
        for (; i < n - 1; ) HSP_LB__(s) \
      }  \
      h; \
    })

// hash_fnv32 inlined seed
//__attribute__((always_inline)) __attribute__((pure))
//static inline uint32_t hsp_hash(const char* s) {
//    uint32_t h = H_SEED;
//    while (*s) {
//        h ^= (uint32_t) *(s++);
//        h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24);
//    }
//    return h;
//}

__attribute__((pure))
__attribute__((always_inline))
static inline uint32_t hsp_hash_inline_(const char* s, uint32_t n) {
    uint32_t i = 0;
    uint32_t h = H_SEED;
    for(; i < n - 1;) HSP_LB__(s)
    return h;
}

#define hsp_hash(S) hash_fnv32((S), H_SEED)

#ifdef NDEBUG

#define H(S) (__builtin_constant_p((S)) ? hsp_hash_inline_(S, sizeof(S)) : hsp_hash(S))

#else

string_hash_t hsp_hash_debug(const char* str, bool literal);
#define H(S) (hsp_hash_debug(S, __builtin_constant_p((S))))

#endif

#ifdef __cplusplus
}
#endif

#endif // EK_HASH_H
