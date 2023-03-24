#pragma once

#include "core.hpp"

#include <ek/math.h>
#include <ek/ds/Array.hpp>

namespace ek {

// Only for dice_twice: TODO: deprecated, remove

template<typename S, typename T>
inline void serialize(IO<S>& io, Array<T>& value) {
    uint32_t size;

    if constexpr (is_readable_stream<S>()) {
        io.value(size);
        value.resize(size);
    } else {
        size = static_cast<uint32_t>(value.size());
        io.value(size);
    }

    if constexpr(is_pod_type<T>()) {
        io.span(value.data(), size * sizeof(T));
    } else {
        for (T& el : value) {
            io(el);
        }
    }
}

#define DEF_POD(T) template<> struct declared_as_pod_type<T> : public std::true_type {}

DEF_POD(ivec2_t);
DEF_POD(ivec3_t);

#undef DEF_POD

}