#ifndef AUPH_NATIVE_DEVICE_IMPL
#define AUPH_NATIVE_DEVICE_IMPL
#else
#error You should implement auph once
#endif

#if defined(__APPLE__) && !defined(AUPH_FORCE_NATIVE_NULL_DEVICE)

#include "device_core_audio.m"

#elif defined(__ANDROID__)

#if defined(AUPH_FORCE_NATIVE_NULL_DEVICE)

#include "device_null.c"

#define OBOE_NULL

#endif

#include "device_oboe.cpp"

#else

#include "device_null.c"

#endif
