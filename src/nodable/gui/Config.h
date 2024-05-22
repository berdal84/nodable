#pragma once

#include <string>
#include <vector>
#include <memory>

#include "fw/core/reflection/reflection"
#include "fw/core/types.h"
#include "fw/gui/AppView.h"
#include "fw/gui/FontManager.h"
#include "fw/gui/ImGuiEx.h"
#include "Isolation.h"
#include "types.h"

namespace ndbl
{
    struct Config
    {
        Config();
        void   reset_default();
        int    ui_grid_subdiv_size() const;

        TextEditor::Palette ui_text_textEditorPalette{};
        fw::Vec2       ui_wire_bezier_roundness; // {min, max}
        float          ui_wire_bezier_thickness;
        fw::Vec2       ui_wire_bezier_fade_length_minmax;
        fw::Vec4       ui_wire_color;
        fw::Vec4       ui_wire_shadowColor;
        float          ui_slot_radius;
        fw::Vec4       ui_slot_border_color;
        fw::Vec4       ui_slot_color;
        fw::Vec4       ui_slot_hovered_color;
        fw::Vec2       ui_slot_size;
        float          ui_slot_gap;
        float          ui_slot_border_radius;
        float          ui_slot_invisible_ratio;
        float          ui_node_spacing;
        fw::Vec4       ui_node_padding; // left, top, right, bottom
        float          ui_node_borderWidth;
        float          ui_node_instructionBorderRatio; // ratio to apply to borderWidth
        fw::Vec4       ui_node_variableColor;
        fw::Vec4       ui_node_invokableColor;
        fw::Vec4       ui_node_instructionColor;
        fw::Vec4       ui_node_literalColor;
        fw::Vec4       ui_node_condStructColor;
        fw::Vec4       ui_node_shadowColor;
        fw::Vec4       ui_node_borderColor;
        fw::Vec4       ui_node_borderHighlightedColor;
        fw::Vec4       ui_node_highlightedColor;
        fw::Vec4       ui_node_fillColor;
        float          ui_node_speed;
        u8_t           ui_node_animation_subsample_count;
        fw::Vec4       ui_codeflow_color;
        fw::Vec4       ui_codeflow_shadowColor;
        float          ui_codeflow_thickness_ratio;
        fw::Vec2       ui_toolButton_size;
        u64_t          ui_history_size_max;
        float          ui_history_btn_spacing;
        float          ui_history_btn_height;
        float          ui_history_btn_width_max;
        const char*    ui_splashscreen_imagePath;
        float          ui_overlay_margin;
        float          ui_overlay_indent;
        fw::Vec4       ui_overlay_window_bg_golor;
        fw::Vec4       ui_overlay_border_color;
        fw::Vec4       ui_overlay_text_color;
        fw::Vec4       ui_graph_grid_color_major;
        fw::Vec4       ui_graph_grid_color_minor;
        i32_t          ui_grid_subdiv_count;
        i32_t          ui_grid_size;
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
        Isolation      isolation;
        float          graph_unfold_dt;
        i32_t          graph_unfold_iterations;
    };

    extern Config& g_conf(); // Global configuration for nodable
}
