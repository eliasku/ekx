#include "SGBuilder.hpp"

#include "AnimationHelpers.hpp"
#include "RenderElement.hpp"

#include "../xfl/Doc.hpp"
#include "../xfl/renderer/Scanner.hpp"
#include <ek/log.h>
#include <ek/assert.h>
#include <stb/stb_sprintf.h>

sg_movie_layer_t* find_target_layer(const sg_movie_t* movie, const sg_node_data_t* item) {
    arr_for(layer, movie->layers) {
        arr_for (target, layer->targets) {
            if (item == *target) {
                return layer;
            }
        }
    }
    return nullptr;
}

sg_node_flags_t get_bounding_rect_flags(const char* name) {
    int flags = 0;
    if (ek_cstr_equals(name, "hitrect", 1)) {
        flags |= SG_NODE_HIT_AREA_ENABLED;
    }
    if (ek_cstr_equals(name, "bbrect", 1)) {
        flags |= SG_NODE_BOUNDS_ENABLED;
    }
    if (ek_cstr_equals(name, "cliprect", 1)) {
        flags |= SG_NODE_SCISSORS_ENABLED;
    }
    return (sg_node_flags_t) flags;
}

namespace ek::xfl {

bool setupSpecialLayer(const Doc& doc, const Layer& layer, ExportItem& toItem) {
    sg_node_flags_t flags = get_bounding_rect_flags(layer.name.c_str());
    if (flags != 0) {
        toItem.node.bounding_rect = Scanner::getBounds(doc, layer.frames[0].elements);
        toItem.node.flags = (sg_node_flags_t) (toItem.node.flags | flags);
        return true;
    }
    return false;
}

void collectFramesMetaInfo(const Doc& doc, ExportItem& item) {
    if (!item.ref) {
        return;
    }
    auto& layers = item.ref->timeline.layers;
    for (auto& layer: layers) {
        if (setupSpecialLayer(doc, layer, item)) {
            continue;
        }
        for (auto& frame: layer.frames) {
            if (!frame.script.empty()) {
                sg_frame_script_t script;
                // TODO: check string life-time?
                script.code = frame.script.c_str();
                script.frame = frame.index;
                arr_push(item.node.scripts, script);
            }
            if (!frame.name.empty()) {
                sg_frame_label_t label;
                // TODO: check string life-time?
                label.name = frame.name.c_str();
                label.frame = frame.index;
                arr_push(item.node.labels, label);
            }
        }
    }
}

bool shouldConvertItemToSprite(ExportItem& item) {
    if (item.children.size() == 1 && item.drawingLayerChild) {
        return true;
    } else if (!arr_empty(item.node.labels) && strcmp(item.node.labels[0].name, "*static") == 0) {
        // special user TAG
        return true;
    } else if (!rect_is_empty(item.node.scale_grid)) {
        // scale 9 grid items
        return true;
    }
    return false;
}

void process_transform(const Element& el, ExportItem& item) {
    item.node.matrix = el.transform.matrix;
    item.node.color = el.transform.color;
    if (el.isVisible) {
        item.node.flags = (sg_node_flags_t) (item.node.flags | SG_NODE_VISIBLE);
    } else {
        item.node.flags = (sg_node_flags_t) (item.node.flags & ~SG_NODE_VISIBLE);
    }
}

// TODO: remove ugly filters
void process_filters(const Element& el, ExportItem& item) {
    (void) (item);
    for (auto& filter: el.filters) {
        sg_filter_t fd;
        fd.type = SG_FILTER_NONE;
        fd.quality = filter.quality;
        fd.color = color_vec4(filter.color);
        fd.blur = filter.blur;
        fd.offset = filter.distance * vec2_cs(to_radians(filter.angle));

        if (filter.type == FilterType::drop_shadow) {
            fd.type = SG_FILTER_SHADOW;
        } else if (filter.type == FilterType::glow) {
            fd.type = SG_FILTER_GLOW;
        }

        if (fd.type != SG_FILTER_NONE) {
            //item.node.filters.push_back(fd);
        }
    }
}

void processTextField(const Element& el, ExportItem& item, const Doc& doc) {
    sg_dynamic_text_t tf = {};
    //if(dynamicText.rect != null) {
//    item->node.matrix.tx += el.rect.x - 2;
//    item->node.matrix.ty += el.rect.y - 2;
    //}
    const auto& textRun = el.textRuns[0];
    auto faceName = textRun.attributes.face;
    if (!faceName.empty() && faceName.back() == '*') {
        faceName.pop_back();
        const auto* fontItem = doc.find(faceName, ElementType::font_item, true);
        if (fontItem) {
            faceName = fontItem->font;
        }
    }
    // animate exports as CR line-ending (legacy MacOS),
    // we need only LF to not check twice when drawing the text
    tf.text = (const char*) malloc(textRun.characters.size() + 1);
    memcpy((void*) tf.text, textRun.characters.data(), textRun.characters.size() + 1);
    ek_cstr_replace((char*) tf.text, '\r', '\n');

    tf.font = H(faceName.c_str());
    tf.size = textRun.attributes.size;
    if (el.lineType == TextLineType::Multiline) {
        tf.word_wrap = true;
    }
    tf.rect = rect_expand(el.rect, 2.0f);
    tf.alignment = textRun.attributes.alignment;
    tf.line_height = textRun.attributes.lineHeight;
    tf.line_spacing = textRun.attributes.lineSpacing;

    sg_text_layer_t layer = {};
    layer.color = color_vec4(textRun.attributes.color);
    arr_push(tf.layers, layer);

    for (auto& filter: el.filters) {
        layer.color = color_vec4(filter.color);
        layer.blur_radius = MIN(filter.blur.x, filter.blur.y);
        layer.blur_iterations = filter.quality;
        layer.offset = vec2(0, 0);
        if (filter.type == FilterType::drop_shadow) {
            layer.offset = filter.distance * vec2_cs(to_radians(filter.angle));
        }
        layer.strength = (int) filter.strength;
        arr_push(tf.layers, layer);
    }
    if (!item.node.dynamic_text) {
        item.node.dynamic_text = (sg_dynamic_text_t*) malloc(sizeof(sg_dynamic_text_t));
    }
    *item.node.dynamic_text = tf;
}

SGBuilder::SGBuilder(const Doc& doc) : doc{doc} {
}

SGBuilder::~SGBuilder() = default;

void SGBuilder::build_library() {

    for (const auto& item: doc.library) {
        process(item, &library);
    }

    for (auto* item: library.children) {
        if (item->ref && item->ref->item.linkageExportForAS) {
            auto& linkageName = item->ref->item.linkageClassName;
            if (!linkageName.empty()) {
                linkages[linkageName] = item->ref->item.name;
            }
            item->inc_ref(library);
            item->update_scale(library, library.node.matrix);
        }
    }

    Array<ExportItem*> chi{};
    for (auto& item: library.children) {
        if (item->usage > 0) {
            chi.push_back(item);
        } else {
            delete item;
        }
    }
    library.children = chi;
}

sg_file_t SGBuilder::export_library() {

    for (auto* item: library.children) {
        // CHANGE: we disable sprite assignment here
//        if (item->ref && (item->shapes > 0 || item->ref->bitmap)) {
//            item->node.sprite = item->node.libraryName;
//        }

        for (auto* child: item->children) {
            arr_push(item->node.children, child->node);
        }

        // if item should be in global registry,
        // but if it's inline sprite - it's ok to throw it away
        if (item->node.library_name) {
            arr_push(library.node.children, item->node);
        }
    }

    for (uint32_t i = 0, len = arr_size(library.node.children); i < len; ++i) {
        sg_node_data_t* item = library.node.children + i;
        for (uint32_t j = 0, len2 = arr_size(item->children); j < len2; ++j) {
            sg_node_data_t* child = item->children + j;
            const ExportItem* ref = library.find_library_item(child->library_name);
            if (ref && ref->node.sprite == ref->node.library_name && rect_is_empty(ref->node.scale_grid)) {
                child->sprite = ref->node.sprite;
                child->library_name = 0;
            }
        }
    }

    // for (const item of this.library.node.children) {
    //     if (item.children.length === 1) {
    //         const child = item.children[0];
    //         if(child.sprite && child.scaleGrid.empty) {
    //             item.sprite = child.sprite;
    //             item.children.length = 0;
    //         }
    //     }
    // }

    sg_file_t sg = {};
    for (auto& pair: linkages) {
        sg_scene_info_t info;
        info.name = H(pair.first.c_str());
        info.linkage = H(pair.second.c_str());
        arr_push(sg.linkages, info);
    }
    for (auto& info: doc.scenes) {
        string_hash_t name_hash = H(info.item.c_str());
        arr_push(sg.scenes, name_hash);
    }

    arr_for(item, library.node.children) {
        if (item->sprite == item->library_name &&
            rect_is_empty(item->scale_grid) &&
            !isInLinkages(item->library_name)) {
            continue;
        }
        arr_push(sg.library, *item);
    }

    return sg;
}

void SGBuilder::process_symbol_instance(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    EK_ASSERT(el.elementType == ElementType::symbol_instance);

    auto* item = new ExportItem();
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = H(el.item.name.c_str());
    item->node.library_name = H(el.libraryItemName.c_str());
    if (el.symbolType == SymbolType::button) {
        item->node.flags = (sg_node_flags_t) (item->node.flags | SG_NODE_BUTTON);
    } else {
        item->node.flags = (sg_node_flags_t) (item->node.flags & ~SG_NODE_BUTTON);
    }
    if (el.silent) {
        item->node.flags = (sg_node_flags_t) (item->node.flags & ~SG_NODE_TOUCHABLE);
    } else {
        item->node.flags = (sg_node_flags_t) (item->node.flags | SG_NODE_TOUCHABLE);
    }

    process_filters(el, *item);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_bitmap_instance(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    EK_ASSERT(el.elementType == ElementType::bitmap_instance);

    auto* item = new ExportItem;
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = H(el.item.name.c_str());
    item->node.library_name = H(el.libraryItemName.c_str());

    process_filters(el, *item);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_bitmap_item(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    auto* item = new ExportItem();
    item->ref = &el;
    item->node.library_name = H(el.item.name.c_str());
    item->renderThis = true;
    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_dynamic_text(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    EK_ASSERT(el.elementType == ElementType::dynamic_text);

    auto* item = new ExportItem();
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = H(el.item.name.c_str());

    processTextField(el, *item, doc);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_symbol_item(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    EK_ASSERT(el.elementType == ElementType::symbol_item ||
              el.elementType == ElementType::scene_timeline);

    auto* item = new ExportItem();
    item->ref = &el;
    process_transform(el, *item);
    item->node.library_name = H(el.item.name.c_str());
    EK_ASSERT(el.libraryItemName.empty());
    item->node.scale_grid = el.scaleGrid;

    collectFramesMetaInfo(doc, *item);

    const auto frames_count = el.timeline.getTotalFrames();
    const auto elements_count = el.timeline.getElementsCount();

    if (shouldConvertItemToSprite(*item)) {
        item->renderThis = true;
        item->children.clear();
    } else {
        const auto withoutTimeline = frames_count <= 1 ||
                                     elements_count == 0 ||
                                     el.item.linkageBaseClass == "flash.display.Sprite" ||
                                     el.item.linkageBaseClass == "flash.display.Shape";

        if (withoutTimeline) {
            const auto& layers = el.timeline.layers;
            for (int layerIndex = int(layers.size()) - 1; layerIndex >= 0; --layerIndex) {
                const auto& layer = layers[layerIndex];
                if (layer.layerType == LayerType::normal) {
                    for (auto& frame: layer.frames) {
                        for (auto& frameElement: frame.elements) {
                            _animationSpan0 = 0;
                            _animationSpan1 = 0;
                            process(frameElement, item);
                        }
                    }
                }
            }
            if (item->children.size() == 1 && item->drawingLayerChild) {
                item->renderThis = true;
                item->children.clear();
            }
        } else {
            processTimeline(el, item);
        }
    }

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_group(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    EK_ASSERT(el.elementType == ElementType::group);
    for (const auto& member: el.members) {
        process(member, parent, bag);
    }
}

void SGBuilder::process_shape(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    EK_ASSERT(el.elementType == ElementType::shape ||
              el.elementType == ElementType::object_oval ||
              el.elementType == ElementType::object_rectangle);
    auto* item = addElementToDrawingLayer(parent, el);
    if (bag) {
        bag->list.push_back(item);
    }
//    if (parent) {
//        parent->shapes++;
//    }
}

// we need global across all libraries to avoid multiple FLA exports overlapping
int NEXT_SHAPE_IDX = 0;

ExportItem* SGBuilder::addElementToDrawingLayer(ExportItem* item, const Element& el) {
    if (item->drawingLayerChild) {
        auto* child = item->drawingLayerChild;
        if (item->children.back() == child &&
            child->drawingLayerItem &&
            child->animationSpan0 == _animationSpan0 &&
            child->animationSpan1 == _animationSpan1) {
            // log_debug("Found drawing layer " << child->ref->item.name);
            auto& timeline = child->drawingLayerItem->timeline;
            EK_ASSERT(!timeline.layers.empty());
            EK_ASSERT(!timeline.layers[0].frames.empty());
            timeline.layers[0].frames[0].elements.push_back(el);
            child->shapes++;
            return child;
        }
    }
    auto shapeItem = std::make_unique<Element>();

    char shapeName[64];
    ek_snprintf(shapeName, sizeof(shapeName), "$%d", ++NEXT_SHAPE_IDX);
    {
        shapeItem->item.name = shapeName;
        shapeItem->elementType = ElementType::symbol_item;
        auto& layer = shapeItem->timeline.layers.emplace_back();
        auto& frame = layer.frames.emplace_back();
        frame.elements.push_back(el);
    }

    auto* layer = new ExportItem();
    layer->ref = shapeItem.get();
    layer->node.library_name = H(shapeName);
    layer->renderThis = true;
    layer->animationSpan0 = _animationSpan0;
    layer->animationSpan1 = _animationSpan1;
    layer->append_to(&library);

    auto shapeInstance = std::make_unique<Element>();
    shapeInstance->libraryItemName = shapeName;
    shapeInstance->elementType = ElementType::symbol_instance;

    processing_bag_t bag;
    process(*shapeInstance, item, &bag);
    auto* drawingLayerInstance = bag.list[0];
    drawingLayerInstance->drawingLayerInstance = std::move(shapeInstance);
    drawingLayerInstance->drawingLayerItem = std::move(shapeItem);
    item->drawingLayerChild = drawingLayerInstance;
    item->shapes++;
    // log_debug("Created drawing layer " << newElement->item.name);
    return drawingLayerInstance;
}

void SGBuilder::process(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    const auto type = el.elementType;
    switch (type) {
        case ElementType::symbol_instance:
            process_symbol_instance(el, parent, bag);
            break;
        case ElementType::bitmap_instance:
            process_bitmap_instance(el, parent, bag);
            break;
        case ElementType::bitmap_item:
            process_bitmap_item(el, parent, bag);
            break;
        case ElementType::symbol_item:
        case ElementType::scene_timeline:
            process_symbol_item(el, parent, bag);
            break;
        case ElementType::dynamic_text:
            process_dynamic_text(el, parent, bag);
            break;
        case ElementType::group:
            process_group(el, parent, bag);
            break;
        case ElementType::shape:
        case ElementType::object_oval:
        case ElementType::object_rectangle:
            process_shape(el, parent, bag);
            break;

        case ElementType::font_item:
        case ElementType::sound_item:
        case ElementType::static_text:
            log_warn("element type is not supported yet: %d", static_cast<int>(type));
            break;

        case ElementType::unknown:
            log_warn("unknown element type: %d", static_cast<int>(type));
            break;
    }
}

/*** rendering ***/

void SGBuilder::render(const ExportItem& item, image_set_t* toImageSet) const {
    const Element& el = *item.ref;
    const auto spriteID = el.item.name;
    RenderElementOptions options;
    for (auto& resolution: toImageSet->resolutions) {
        options.scale = std::min(
                item.max_abs_scale,
                resolution.scale * std::min(1.0f, item.estimated_scale)
        );
        sprite_data_t res = renderElement(doc, el, options);

        // copy string: res.name = spriteID.c_str();
        arr_init_from((void**) &res.name, 1, spriteID.c_str(), spriteID.size() + 1);

        res.trim = rect_is_empty(item.node.scale_grid);
        arr_push(resolution.sprites, res);
    }
}

void SGBuilder::build_sprites(image_set_t* toImageSet) const {
    for (auto* item: library.children) {
        if (item->renderThis) {
            item->node.sprite = H(item->ref->item.name.c_str());
            render(*item, toImageSet);
        }
        if (!rect_is_empty(item->node.scale_grid)) {

        }
    }
}

bool SGBuilder::isInLinkages(const string_hash_t id) const {
    for (const auto& pair: linkages) {
        if (H(pair.second.c_str()) == id) {
            return true;
        }
    }
    return false;
}

void SGBuilder::processTimeline(const Element& el, ExportItem* item) {
    sg_movie_t movie = {};
    movie.frames = el.timeline.getTotalFrames();
    movie.fps = doc.info.frameRate;

    const auto& layers = el.timeline.layers;
    for (int layerIndex = int(layers.size()) - 1; layerIndex >= 0; --layerIndex) {
        auto& layer = layers[layerIndex];
        // ignore other layers.
        // TODO: mask layer
        if (layer.layerType != LayerType::normal) {
            continue;
        }
        const int framesTotal = static_cast<int>(layer.frames.size());
        for (int frameIndex = 0; frameIndex < framesTotal; ++frameIndex) {
            auto& frame = layer.frames[frameIndex];
            processing_bag_t targets;
            for (const auto& frameElement: frame.elements) {
                bool found = false;
                for (auto* prevItem: item->children) {
                    if (prevItem->ref &&
                        prevItem->ref->libraryItemName == frameElement.libraryItemName &&
                        prevItem->ref->item.name == frameElement.item.name &&
                        prevItem->fromLayer == layerIndex &&
                        prevItem->movieLayerIsLinked) {
                        targets.list.push_back(prevItem);
                        // copy new transform
                        prevItem->ref = &frameElement;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    _animationSpan0 = frame.index;
                    _animationSpan1 = frame.endFrame();
                    process(frameElement, item, &targets);
                }
            }
            const sg_movie_frame_t k0 = createFrameModel(frame);
            sg_keyframe_transform_t delta = {};
            bool has_delta = false;
            if (k0.motion_type == 1
                && !frame.elements.empty()
                && (frameIndex + 1) < framesTotal) {
                const auto& nextFrame = layer.frames[frameIndex + 1];
                if (!nextFrame.elements.empty()) {
                    const auto& el0 = frame.elements.back();
                    const auto& el1 = nextFrame.elements[0];
                    delta = extractTweenDelta(frame, el0, el1);
                    has_delta = true;
                }
            }
            for (auto* target: targets.list) {
                if (target->ref) {
                    sg_node_data_t* targetNodeRef = &target->node;
                    sg_movie_layer_t* targetLayer = nullptr;
                    if (!target->movieLayerIsLinked) {
                        sg_movie_layer_t newLayer = {};
                        arr_push(newLayer.targets, targetNodeRef);
                        arr_push(movie.layers, newLayer);
                        targetLayer = movie.layers + arr_size(movie.layers) - 1;
                        target->fromLayer = layerIndex;
                        target->movieLayerIsLinked = true;
                    } else {
                        targetLayer = find_target_layer(&movie, targetNodeRef);
                        EK_ASSERT(targetLayer);
                    }

                    sg_movie_frame_t kf0 = createFrameModel(frame);
                    setupFrameFromElement(kf0, *target->ref);
                    arr_push(targetLayer->frames, kf0);
                    if (has_delta) {
                        sg_movie_frame_t kf1 = {};
                        kf1.easing = nullptr;
                        kf1.index = kf0.index + kf0.duration;
                        //kf1.duration = 0;
                        kf1.transform = add_keyframe_transform(&kf0.transform, &delta);
                        //kf1.visible = false;
                        arr_push(targetLayer->frames, kf1);
                    }
                }
            }
        }

        for (sg_movie_layer_t* it = movie.layers, * end = arr_end(movie.layers); it != end;) {
            uint32_t frames_num = arr_size(it->frames);
            bool empty = frames_num == 0;
            if (frames_num == 1) {
                sg_movie_frame_ frame = it->frames[0];
                if (frame.index == 0 && frame.motion_type != 2 && frame.duration == movie.frames) {
                    empty = true;
                }
            }
            if (empty) {
                arr_erase(movie.layers, it);
                --end;
            } else {
                ++it;
            }
        }

        if (movie.frames > 1 && !arr_empty(movie.layers)) {
            uint32_t layers_count = arr_size(movie.layers);
            for (uint32_t i = 0; i < layers_count; ++i) {
                sg_node_data_t** targets = movie.layers[i].targets;
                for (sg_node_data_t** it = targets, ** end = targets + arr_size(targets); it != end; ++it) {
                    (*it)->movie_target_id = static_cast<int>(i);
                }
            }
            if (!item->node.movie) {
                item->node.movie = (sg_movie_t*) malloc(sizeof(sg_movie_t));
            }
            *(item->node.movie) = movie;
        }

        _animationSpan0 = 0;
        _animationSpan1 = 0;
    }
}

}