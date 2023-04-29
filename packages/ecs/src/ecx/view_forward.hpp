#pragma once

#include "world.hpp"

namespace ecs {

template<typename ...Cn>
class ViewForward {
public:
    static constexpr auto components_num = sizeof ... (Cn);

    class iterator final {
    public:
        iterator(const ecx_component_type* const table[components_num], uint32_t it) noexcept: it_{it},
                                                                                               table_{table} {
            if (!is_valid(it_, table_)) {
                ++(*this);
            }
        }

        iterator() noexcept = default;

        inline iterator& operator++() noexcept {
            return is_valid(++it_, table_) ? *this : ++(*this);
        }

        const iterator operator++(int) noexcept {
            iterator orig = *this;
            return ++(*this), orig;
        }

        inline bool operator==(const iterator& other) const {
            return it_ == other.it_;
        }

        inline bool operator!=(const iterator& other) const {
            return it_ != other.it_;
        }

        inline static bool is_valid(uint32_t it, const ecx_component_type* const * table) {
            // check primary entity vector end
            const ecx_component_type* m = table[0];
            if (it == m->size) {
                return true;
            }
            // filter secondary entity vectors
            const entity_idx_t entity_idx = m->handle_to_entity[it];
            for (uint32_t k = 1; k < components_num; ++k) {
                if (get_component_handle_by_index(table[k], entity_idx) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline static bool is_valid_fast(entity_idx_t entity_idx, const ecx_component_type** table) {
            // filter secondary entity vectors
            const uint32_t cn = components_num;
            for (uint32_t k = 1; k < cn; ++k) {
                if (get_component_handle_by_index(table[k], entity_idx) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline entity_t operator*() const noexcept {
            return entity_at(table_[0]->handle_to_entity[it_]);
        }

        inline entity_t operator*() noexcept {
            return entity_at(table_[0]->handle_to_entity[it_]);
        }

    private:
        uint32_t it_ = 0;
        const ecx_component_type* const* table_;
    };

    ViewForward() noexcept {
        {
            uint32_t i = 0;
            ((table_[i] = type<Cn>(), ++i), ...);
        }
        ecx_sort_component_type_table(table_, components_num);
#ifndef NDEBUG
        {
            uint32_t i = 0;
            ((++type<Cn>()->lock_counter, ++i), ...);
        }
#endif
    }

    ~ViewForward() noexcept {
#ifndef NDEBUG
        uint32_t i = 0;
        ((--type<Cn>()->lock_counter, ++i), ...);
#endif
    }

    iterator begin() const {
        return {table_, 1u};
    }

    iterator end() const {
        return {table_, table_[0]->size};
    }

    ecx_component_type* table_[components_num];
};

template<typename C>
class ViewForward<C> {
public:
    static constexpr auto components_num = 1;

    class iterator final {
    public:
        iterator(component_handle_t it) noexcept: it_{it} {
        }

        iterator() noexcept = default;

        inline iterator& operator++() noexcept {
            ++it_;
            return *this;
        }

        const iterator operator++(int) noexcept {
            iterator orig = *this;
            return ++(*this), orig;
        }

        inline bool operator==(const iterator& other) const {
            return it_ == other.it_;
        }

        inline bool operator!=(const iterator& other) const {
            return it_ != other.it_;
        }

        inline entity_t operator*() const noexcept {
            return entity_at(type<C>()->handle_to_entity[it_]);
        }

    private:
        component_handle_t it_{};
    };

    ViewForward() noexcept {
#ifndef NDEBUG
        ++type<C>()->lock_counter;
#endif
    }

    ~ViewForward() noexcept {
#ifndef NDEBUG
        --type<C>()->lock_counter;
#endif
    }

    iterator begin() const {
        return {1u};
    }

    iterator end() const {
        return {type<C>()->size};
    }
};

}