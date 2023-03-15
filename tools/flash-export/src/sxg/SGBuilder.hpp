#pragma once

#include "ExportItem.hpp"
#include "../image_set.h"
#include <unordered_map>
#include <ek/ds/String.hpp>

namespace ek::xfl {

class Doc;

class SGBuilder {
public:
    const Doc& doc;
    ExportItem library;

    // Map export public linkage class name to internal symbol name
    std::unordered_map<String, String> linkages;

private:
    int _animationSpan0 = 0;
    int _animationSpan1 = 0;

public:
    explicit SGBuilder(const Doc& doc);

    ~SGBuilder();

    SGBuilder(const SGBuilder& v) = delete;

    SGBuilder& operator=(const SGBuilder& v) = delete;

    SGBuilder(SGBuilder&& v) = delete;

    SGBuilder& operator=(SGBuilder&& v) = delete;

    void build_library();

    void build_sprites(image_set_t* toImageSet) const;

    void process(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_symbol_instance(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_symbol_item(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_bitmap_instance(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_bitmap_item(const Element& el, ExportItem* library, processing_bag_t* bag = nullptr);

    void process_dynamic_text(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_group(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_shape(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void render(const ExportItem& item, image_set_t* toImageSet) const;

    [[nodiscard]]
    sg_file_t export_library();

    ExportItem* addElementToDrawingLayer(ExportItem* item, const Element& el);

private:
    [[nodiscard]] bool isInLinkages(string_hash_t id) const;

    void processTimeline(const Element& Element, ExportItem* PItem);
};

}