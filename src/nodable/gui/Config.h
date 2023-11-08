#pragma once

#include <string>
#include <vector>
#include <memory>
#include <ImGuiColorTextEdit/TextEditor.h>

#include "fw/core/reflection/reflection"
#include "fw/core/types.h"
#include "fw/gui/Config.h"
#include "fw/gui/FontManager.h"
#include "fw/gui/AppView.h"
#include "nodable/gui/types.h"

namespace ndbl {

    class Config  {
    public:

        Config();
        Config(const Config&) = delete; // Disable copy

        TextEditor::Palette ui_text_textEditorPalette{};
        float          ui_wire_bezier_roundness;
        float          ui_wire_bezier_thickness;
        ImVec2         ui_wire_bezier_fade_length_minmax;
        ImVec4         ui_wire_color;
        ImVec4         ui_wire_shadowColor;
        float          ui_node_propertyslot_radius;
        ImVec4         ui_node_padding; // left, top, right, bottom
        float          ui_node_instructionBorderWidth;
        float          ui_node_borderWidth;
        ImVec4         ui_node_variableColor;
        ImVec4         ui_node_invokableColor;
        ImVec4         ui_node_instructionColor;
        ImVec4         ui_node_literalColor;
        ImVec4         ui_node_condStructColor;
        ImVec4         ui_node_shadowColor;
        ImVec4         ui_node_borderColor;
        ImVec4         ui_node_borderHighlightedColor;
        ImVec4         ui_node_slot_border_color;
        ImVec4         ui_node_highlightedColor;
        ImVec4         ui_node_fillColor;
        ImVec4         ui_node_slot_color;
        ImVec4         ui_node_slot_hovered_color;
        float          ui_node_spacing;
        float          ui_node_speed;
        u8_t           ui_node_animation_subsample_count;
        float          ui_node_slot_height;
        float          ui_node_slot_padding;
        float          ui_node_slot_width;
        float          ui_node_slot_border_radius;
        ImVec4         ui_codeflow_color;
        ImVec4         ui_codeflow_shadowColor;
        float          ui_codeflow_thickness_ratio;
        ImVec2         ui_toolButton_size;
        float          ui_history_btn_spacing;
        float          ui_history_btn_height;
        float          ui_history_btn_width_max;
        const char*    ui_splashscreen_imagePath;
        float          ui_overlay_margin;
        float          ui_overlay_indent;
        ImVec4         ui_overlay_window_bg_golor;
        ImVec4         ui_overlay_border_color;
        ImVec4         ui_overlay_text_color;
        ImVec4         ui_graph_grid_color_major;
        ImVec4         ui_graph_grid_color_minor;
        i32_t          ui_graph_grid_subdivs;
        i32_t          ui_graph_grid_size;
        const char*    ui_file_info_window_label;
        const char*    ui_help_window_label;
        const char*    ui_imgui_config_window_label;
        const char*    ui_node_properties_window_label;
        const char*    ui_config_window_label;
        const char*    ui_startup_window_label;
        const char*    ui_toolbar_window_label ;
        const char*    ui_virtual_machine_window_label;
        bool           experimental_graph_autocompletion;
        bool           experimental_hybrid_history;
        bool           isolate_selection;
        float          graph_unfold_dt;
        i32_t          graph_unfold_iterations;
        fw::Config common;
    };
}
