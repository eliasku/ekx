#include "ExportItem.hpp"
#include "../xfl/types.hpp"
#include <ek/assert.h>

namespace ek::xfl {

ExportItem::~ExportItem() {
    for (auto ch : children) {
        delete ch;
    }
}

void ExportItem::add(ExportItem* item) {
    EK_ASSERT(item != nullptr);
    children.push_back(item);
    item->parent = this;
}

void ExportItem::append_to(ExportItem* parent_) {
    EK_ASSERT(parent == nullptr);
    if (parent_) {
        parent_->add(this);
    }
}

ExportItem* ExportItem::find_library_item(const string_hash_t libraryName) const {
    for (auto& child : children) {
        if (child->node.library_name == libraryName) {
            return child;
        }
    }
    return nullptr;
}

void ExportItem::inc_ref(ExportItem& lib) {
    ++usage;
    if (node.library_name) {
        auto* dependency = lib.find_library_item(node.library_name);
        if (dependency && dependency != this) {
            dependency->inc_ref(lib);
        }
    }
    // else it's something like dynamic_text?
//    else {
//        EK_WARN << ref->item.linkageClassName;
//    }
    for (auto* child : children) {
        child->inc_ref(lib);
    }
}

void ExportItem::update_scale(ExportItem& lib, const mat3x2_t parent_matrix) {
    if (!rect_is_empty(node.scale_grid)) {
        estimated_scale = 1.0f;
        return;
    }

    const auto global_matrix = mat3x2_mul(parent_matrix, node.matrix);
    const auto scale = mat3x2_get_scale(global_matrix);
    const auto s = std::max(scale.x, scale.y);
    estimated_scale = std::max(s, estimated_scale);
    if (ref && ref->elementType == ElementType::bitmap_item) {
        max_abs_scale = 1.0f;
    }

    if (node.library_name) {
        auto* dependency = lib.find_library_item(node.library_name);
        if (dependency && dependency != this) {
            dependency->update_scale(lib, global_matrix);
        }
    }
    for (auto& child : children) {
        child->update_scale(lib, global_matrix);
    }
}

}