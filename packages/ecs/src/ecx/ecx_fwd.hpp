#pragma once

#include "ecx.h"
#include <cstddef>
//
//constexpr bool operator==(struct entity_t a, struct entity_t b) { return a.id == b.id; }
//
//constexpr bool operator!=(struct entity_t a, struct entity_t b) { return a.id != b.id; }

namespace ecs {

template<typename ...Component>
class ViewForward;

template<typename ...Component>
class ViewBackward;

}
