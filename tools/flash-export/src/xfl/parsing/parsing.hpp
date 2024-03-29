#pragma once

#include <cstdint>
#include <string>
#include <ek/math.h>

namespace pugi {

class xml_node;

class xml_document;

}

namespace ek::xfl {

using xml_node = pugi::xml_node;
using xml_document = pugi::xml_document;

template<typename T>
T parse_xml_node(const xml_node& node) {
    T r;
    r << node;
    return r;
}

rect_t read_rect(const xml_node& node);

rect_t read_scale_grid(const xml_node& node);

vec2_t read_point(const xml_node& node);

vec2_t read_transformation_point(const xml_node& node);

mat3x2_t& operator<<(mat3x2_t& r, const xml_node& node);

color2f_t& operator<<(color2f_t& color, const xml_node& node);

enum class BlendMode;
enum class ScaleMode;
enum class SolidStyleType;
enum class LineCaps;
enum class LineJoints;
enum class FillType;
enum class SpreadMethod;
enum class FilterType;
enum class TweenType;
enum class RotationDirection;
enum class LayerType;
enum class ElementType;
enum class SymbolType;
enum class LoopMode;
enum class TweenTarget;
enum class FontRenderingMode;
enum class TextLineType;

struct ItemProperties;
struct Element;
struct FolderItem;
struct DocInfo;
struct GradientEntry;
struct FillStyle;
struct StrokeStyle;
struct Filter;
struct MotionObject;
struct TextAttributes;
struct TextRun;
struct Element;
struct Frame;
struct Layer;
struct Timeline;
struct Edge;

class FileNode;

TweenTarget& operator<<(TweenTarget& r, const char* str);

FontRenderingMode& operator<<(FontRenderingMode& r, const char* str);

TextLineType& operator<<(TextLineType& r, const char* str);

BlendMode& operator<<(BlendMode& r, const char* str);

ScaleMode& operator<<(ScaleMode& r, const char* str);

SolidStyleType& operator<<(SolidStyleType& r, const char* str);

LineCaps& operator<<(LineCaps& r, const char* str);

LineJoints& operator<<(LineJoints& r, const char* str);

FillType& operator<<(FillType& r, const char* str);

SpreadMethod& operator<<(SpreadMethod& r, const char* str);

GradientEntry& operator<<(GradientEntry& r, const xml_node& node);

FilterType& operator<<(FilterType& r, const char* str);

Filter& operator<<(Filter& r, const xml_node& node);

TweenType& operator<<(TweenType& r, const char* str);

RotationDirection& operator<<(RotationDirection& r, const char* str);

TextAttributes& operator<<(TextAttributes& r, const xml_node& node);

TextRun& operator<<(TextRun& r, const xml_node& node);

LayerType& operator<<(LayerType& r, const char* str);

ElementType& operator<<(ElementType& r, const char* str);

SymbolType& operator<<(SymbolType& r, const char* str);

LoopMode& operator<<(LoopMode& r, const char* str);

ItemProperties& operator<<(ItemProperties& r, const xml_node& node);

FolderItem& operator<<(FolderItem& r, const xml_node& node);

DocInfo& operator<<(DocInfo& r, const xml_node& node);

Edge& operator<<(Edge& r, const xml_node& node);

}


