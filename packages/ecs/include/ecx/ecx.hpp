#pragma once

#include "ecx.h"
#include "world.hpp"
#include "view_forward.hpp"
#include "view_backward.hpp"

namespace ecs {

/**
 * NULL entity value
 */

template<typename ...Cn>
inline auto view() {
    return ViewForward<Cn...>();
}

template<typename C>
inline void foreach(void(* callback)(component_handle_t handle)) {
    foreach_type(type<C>(), callback);
}

template<typename C>
inline entity_t get_entity(component_handle_t handle) {
    return get_entity(type<C>(), handle);
}

template<typename C, unsigned Index = 0, typename DataType = C>
inline DataType* get_data(component_handle_t handle) {
    return (DataType*) type<C>()->data[Index] + handle;
}

template<typename C>
inline component_handle_t get_handle(entity_t entity) {
    return get_component_handle(type<C>(), entity);
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Cn>
inline auto view_backward() {
    return ViewBackward<Cn...>();
}

}

#ifndef ECX_COMPONENT

#define ECX_COMPONENT(C) ecs::ComponentType<C>::setup(4, ECX_TYPE_LABEL__(C))
#define ECX_COMP_TYPE_CXX(C) template<> class ecs::ComponentType<C> final : public ecs::GenericComponent<C, 3> {};

#endif // !ECX_COMPONENT
