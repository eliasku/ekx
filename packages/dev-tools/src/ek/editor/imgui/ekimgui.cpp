#include "ekimgui.h"

#include <imgui.h>
#include <implot.h>
#include <cmath>
#include <fonts/IconsFontAwesome5.h>

#define SOKOL_IMGUI_IMPL
#define SOKOL_IMGUI_NO_SOKOL_APP

#include <sokol/util/sokol_imgui.h>

#define SOKOL_GFX_IMGUI_IMPL

#include <sokol/util/sokol_gfx_imgui.h>

void reset_keys() {
    ImGuiIO& io = ImGui::GetIO();
    for (uint32_t i = 0; i < ImGuiKey_COUNT; ++i) {
        io.KeysDown[i] = false;
    }
}

sg_imgui_t sokol_gfx_gui_state;
char clipboard_text_[1024];
sg_image font_image;

#define DPI_SCALE (ek_app.viewport.scale)

static
void add_font_with_icons(const char* filepath, float dpi_scale) {
    ImGuiIO& io = ImGui::GetIO();
    float pxSize = (13.0f * dpi_scale) * 96.0f / 72.0f;
    float fontSizeScaled = pxSize;
    // MD
    //float iconSizeScaled = pxSize * 1.1f;
    // FA
    float iconSizeScaled = pxSize * 0.75f;
    float fontScale = 1.0f / dpi_scale;
    {
        ImFontConfig fontCfg{};
        fontCfg.OversampleH = 1;
        fontCfg.OversampleV = 1;
        fontCfg.PixelSnapH = true;
        fontCfg.FontDataOwnedByAtlas = false;
        ImFont* font = io.Fonts->AddFontFromFileTTF(filepath, fontSizeScaled, &fontCfg);
        font->Scale = fontScale;
    }

    static const ImWchar iconsRange[3] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    {
        ImFontConfig fontCfg = INIT_ZERO;
        fontCfg.OversampleH = 1;
        fontCfg.OversampleV = 1;
        fontCfg.PixelSnapH = true;
        fontCfg.FontDataOwnedByAtlas = false;
        fontCfg.GlyphMinAdvanceX = fontSizeScaled; // <-- _FONT_SizeScaled!
        // MD
        //fontCfg.GlyphOffset.y = floorf(0.27f * iconSizeScaled);
        // FA
        fontCfg.GlyphOffset.y = floorf(0.05f * iconSizeScaled);
        fontCfg.MergeMode = true;
        ImFont* font = io.Fonts->AddFontFromFileTTF("dev/" FONT_ICON_FILE_NAME_FAS,
                                                    iconSizeScaled, &fontCfg, iconsRange);
        font->Scale = fontScale;
    }
}

static
void init_font_image(float dpi_scale) {
    ImGuiIO& io = ImGui::GetIO();
    add_font_with_icons("dev/sf-pro-text-regular.ttf", dpi_scale);
    add_font_with_icons("dev/sf-mono-text-regular.ttf", dpi_scale);

    uint8_t* pixels;
    int width;
    int height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    sg_image_desc img_desc = INIT_ZERO;
    img_desc.width = width;
    img_desc.height = height;
    img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    img_desc.min_filter = SG_FILTER_LINEAR;
    img_desc.mag_filter = SG_FILTER_LINEAR;
    img_desc.data.subimage[0][0].ptr = pixels;
    img_desc.data.subimage[0][0].size = width * height * 4;
    img_desc.label = "sokol-imgui-font";
    font_image = sg_make_image(&img_desc);

    ImGui::GetIO().Fonts->TexID = (ImTextureID) (uintptr_t) font_image.id;
}

static
void set_clipboard_callback(void* context, const char* text) {
    clipboard_text_[0] = 0;
    strncat(clipboard_text_, text, (sizeof clipboard_text_) - 1);
    // TODO: platform clipboard
    //c->_platform->clipboard.set(text);
}

static
const char* get_clipboard_callback(void* context) {
    return clipboard_text_;
}

#define MAP_KEY_CODE(from, to) io.KeyMap[(to)] = (int)((from))

void ekimgui_setup(void) {
    IMGUI_CHECKVERSION();

    const sg_imgui_desc_t imgui_desc = INIT_ZERO;
    sg_imgui_init(&sokol_gfx_gui_state, &imgui_desc);

    simgui_desc_t desc = INIT_ZERO;
    desc.depth_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    desc.sample_count = 0;
    desc.ini_filename = "imgui.ini";
    desc.no_default_font = true;
    simgui_setup(&desc);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;  // Enable set mouse pos for navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup back-end capabilities flags
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors |// We can honor GetMouseCursor() values (optional)
                       ImGuiBackendFlags_HasSetMousePos; // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    MAP_KEY_CODE(EK_KEYCODE_TAB, ImGuiKey_Tab);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_LEFT, ImGuiKey_LeftArrow);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_RIGHT, ImGuiKey_RightArrow);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_UP, ImGuiKey_UpArrow);
    MAP_KEY_CODE(EK_KEYCODE_ARROW_DOWN, ImGuiKey_DownArrow);
    MAP_KEY_CODE(EK_KEYCODE_PAGEUP, ImGuiKey_PageUp);
    MAP_KEY_CODE(EK_KEYCODE_PAGEDOWN, ImGuiKey_PageDown);
    MAP_KEY_CODE(EK_KEYCODE_HOME, ImGuiKey_Home);
    MAP_KEY_CODE(EK_KEYCODE_END, ImGuiKey_End);
    MAP_KEY_CODE(EK_KEYCODE_INSERT, ImGuiKey_Insert);
    MAP_KEY_CODE(EK_KEYCODE_DELETE, ImGuiKey_Delete);
    MAP_KEY_CODE(EK_KEYCODE_BACKSPACE, ImGuiKey_Backspace);
    MAP_KEY_CODE(EK_KEYCODE_SPACE, ImGuiKey_Space);
    MAP_KEY_CODE(EK_KEYCODE_ENTER, ImGuiKey_Enter);
    MAP_KEY_CODE(EK_KEYCODE_ESCAPE, ImGuiKey_Escape);
    MAP_KEY_CODE(EK_KEYCODE_A, ImGuiKey_A);
    MAP_KEY_CODE(EK_KEYCODE_C, ImGuiKey_C);
    MAP_KEY_CODE(EK_KEYCODE_V, ImGuiKey_V);
    MAP_KEY_CODE(EK_KEYCODE_X, ImGuiKey_X);
    MAP_KEY_CODE(EK_KEYCODE_Y, ImGuiKey_Y);
    MAP_KEY_CODE(EK_KEYCODE_Z, ImGuiKey_Z);

    io.SetClipboardTextFn = set_clipboard_callback;
    io.GetClipboardTextFn = get_clipboard_callback;

    init_font_image(DPI_SCALE);

    ImGui::StyleColorsDark();
    ImGui::GetStyle().Alpha = 0.8f;

    ImPlot::CreateContext();
}

void ekimgui_shutdown(void) {
    ImPlot::DestroyContext();
    sg_imgui_discard(&sokol_gfx_gui_state);
    simgui_shutdown();
}

void ekimgui_on_event(ek_app_event event) {
    ImGuiIO& io = ImGui::GetIO();
    float dpi_scale = DPI_SCALE;
    switch (event.type) {
        case EK_APP_EVENT_KEY_UP:
        case EK_APP_EVENT_KEY_DOWN:
        case EK_APP_EVENT_KEY_PRESS: {
            int key = static_cast<int>(event.key.code);
            if (key >= 0 && key < IM_ARRAYSIZE(io.KeysDown)) {
                io.KeysDown[key] = (event.type == EK_APP_EVENT_KEY_DOWN);
            }

            // update modifier keys
            const bool isShift = (event.key.modifiers & EK_KEY_MOD_SHIFT) != 0;
            const bool isControl = (event.key.modifiers & EK_KEY_MOD_CONTROL) != 0;
            const bool isAlt = (event.key.modifiers & EK_KEY_MOD_ALT) != 0;
            const bool isSuper = (event.key.modifiers & EK_KEY_MOD_SUPER) != 0;
            if ((io.KeyShift && !isShift) || (io.KeyCtrl && !isControl) || (io.KeyAlt && !isAlt) ||
                (io.KeySuper && !isSuper)) {
                // need to reset key states when any of meta keys disabled
                reset_keys();
            }
            io.KeyShift = isShift;
            io.KeyCtrl = isControl;
            io.KeyAlt = isAlt;
            io.KeySuper = isSuper;
        }
            break;

        case EK_APP_EVENT_TEXT:
            if (*event.text.data /* not empty */) {
                io.AddInputCharactersUTF8(event.text.data);
            }
            break;

        case EK_APP_EVENT_MOUSE_DOWN:
        case EK_APP_EVENT_MOUSE_UP: {
            int button = 0;
            if (event.mouse.button == EK_MOUSE_BUTTON_RIGHT) {
                button = 1;
            } else if (event.mouse.button == EK_MOUSE_BUTTON_OTHER) {
                button = 2;
            }
            io.MouseDown[button] = (event.type == EK_APP_EVENT_MOUSE_DOWN);
        }
            break;
        case EK_APP_EVENT_WHEEL:
            if (fabs(event.wheel.x) > 0.0f) {
                io.MouseWheelH += event.wheel.x * 0.1f;
            }
            if (fabs(event.wheel.y) > 0.0f) {
                io.MouseWheel += event.wheel.y * 0.1f;
            }
            break;

        case EK_APP_EVENT_MOUSE_MOVE:
            io.MousePos.x = event.mouse.x / dpi_scale;
            io.MousePos.y = event.mouse.y / dpi_scale;
            break;

        case EK_APP_EVENT_PAUSE:
        case EK_APP_EVENT_RESUME:
            break;

        default:
            break;
    }

}

static
void update_mouse_cursor(void) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) {
        return;
    }

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    ek_mouse_cursor cursor = EK_MOUSE_CURSOR_PARENT;
    bool cursor_visible = true;
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None) {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        cursor_visible = false;
    } else {
        // Show OS mouse cursor
        switch (imgui_cursor) {
            case ImGuiMouseCursor_Arrow:
            case ImGuiMouseCursor_TextInput:
            case ImGuiMouseCursor_ResizeAll:
            case ImGuiMouseCursor_ResizeNS:
            case ImGuiMouseCursor_ResizeEW:
            case ImGuiMouseCursor_ResizeNESW:
            case ImGuiMouseCursor_ResizeNWSE:
                cursor = EK_MOUSE_CURSOR_ARROW;
                break;
            case ImGuiMouseCursor_Hand:
                cursor = EK_MOUSE_CURSOR_BUTTON;
                break;
            default:
                break;
        }
        ek_app_set_mouse_cursor(cursor);
        cursor_visible = true;
    }

    // TODO: cursor visible
    //g_window.hideCursor()->mouse.show(cursorVisible);
}

void ekimgui_begin_frame(float dt) {
    auto w = (int) ek_app.viewport.width;
    auto h = (int) ek_app.viewport.height;
    if (w > 0 && h > 0) {
        update_mouse_cursor();
        simgui_frame_desc_t desc;
        desc.width = w;
        desc.height = h;
        desc.delta_time = dt;
        desc.dpi_scale = DPI_SCALE;
        simgui_new_frame(&desc);
    }
}

void ekimgui_end_frame(void) {
    sg_imgui_draw(&sokol_gfx_gui_state);
    simgui_render();
}

void ekimgui_on_frame_completed(void) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeySuper || io.KeyCtrl) {
        reset_keys();
    }
}

