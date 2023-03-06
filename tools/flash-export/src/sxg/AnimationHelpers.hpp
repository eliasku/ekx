#pragma once

struct sg_keyframe_transform;

namespace ek {
struct SGMovieFrameData;
}

namespace ek::xfl {

struct Frame;
struct Element;

sg_keyframe_transform extractTweenDelta(const Frame& frame, const Element& el0, const Element& el1);

SGMovieFrameData createFrameModel(const Frame& frame);

void setupFrameFromElement(SGMovieFrameData& target, const Element& el);

}

