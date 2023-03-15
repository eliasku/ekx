#pragma once

#include "gen_sg.h"

namespace ek::xfl {

struct Frame;
struct Element;

sg_keyframe_transform_t extractTweenDelta(const Frame& frame, const Element& el0, const Element& el1);

sg_movie_frame_t createFrameModel(const Frame& frame);

void setupFrameFromElement(sg_movie_frame_t& target, const Element& el);

}

