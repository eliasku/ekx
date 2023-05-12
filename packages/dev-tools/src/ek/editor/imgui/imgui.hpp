#pragma once

//#include <ek/debug.hpp>
//#define IM_ASSERT_USER_ERROR(_EXP,_MSG) log_warn(_MSG)

#ifndef EK_DEV_TOOLS_DEBUG
#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_DEBUG_TOOLS
#endif // !EK_DEV_TOOLS_DEBUG

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#ifdef __cplusplus
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
#endif
