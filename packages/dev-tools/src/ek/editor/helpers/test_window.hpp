#pragma once

#include <ek/editor/imgui/cimgui.h>

struct gain_bias_params_t {
    float bias;
    bool gain;
};

struct back_params_t {
    float overshoot;
};

static float graph_biasf(void* data, int idx) {
    auto* params = (gain_bias_params_t*)data;
    const float t = (float)idx / 100.0f;
    return params->gain ? gain_f32(t, params->bias) : bias_f32(t, params->bias);
}

static float graph_back(void* data, int idx) {
    auto* params = (back_params_t*)data;
    const float t = (float)idx / 100.0f;
    return ease_back(t, params->overshoot);
}

void plot_linear() {
    static float x_data[100 + 1];
    static float y_data[100 + 1];
    for (int i = 0; i <= 100; ++i) {
        const float t = (float)i / 100.0f;
        x_data[i] = t;
        y_data[i] = t;
        ImPlot::PlotLine("linear", x_data, y_data, 100 + 1);
    }
}

static void draw_test_window(void) {
    static gain_bias_params_t gain_bias = (gain_bias_params_t){0.5f, true};
    ImGui::DragFloat("Bias", &gain_bias.bias, 0.01f, 0.01f, 0.99f);
    ImGui::Checkbox("Gain", &gain_bias.gain);
    static back_params_t back_params = (back_params_t){1.70158f};
    ImGui::DragFloat("Back Overshoot", &back_params.overshoot, 0.01f, 0.0f, 10.0f);
    if (ImGui_Button("Reset##back_overshoot")) {
        back_params = {};
    }
    if (ImPlot::BeginPlot("Gain Bias", ImVec2(300, 300))) {
        ImPlot::SetupAxes("x", "f(x)");
        plot_linear();
        {
            float x_data[100 + 1];
            float y_data[100 + 1];
            for (int i = 0; i <= 100; ++i) {
                const float t = (float)i / 100.0f;
                x_data[i] = t;
                y_data[i] = gain_bias.gain ? gain_f32(t, gain_bias.bias) : bias_f32(t, gain_bias.bias);
            }
            ImPlot::PlotLine("bias", x_data, y_data, 100 + 1);
        }
        {
            float x_data[100 + 1];
            float y_data[100 + 1];
            for (int i = 0; i <= 100; ++i) {
                const float t = (float)i / 100.0f;
                x_data[i] = t;
                y_data[i] = ease_back(t, back_params.overshoot);
            }
            ImPlot::PlotLine("back", x_data, y_data, 100 + 1);
        }
        ImPlot::EndPlot();
    }
}

void test_window(bool* opened) {
    if (ImGui_Begin("Tests Window###TestWindow", opened, 0)) {
        draw_test_window();
    }
    ImGui_End();
}
