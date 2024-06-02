#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Isolation.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"
#include "tools/gui/AppView.h"
#include "tools/gui/Config.h"
#include "tools/gui/FontManager.h"
#include "tools/gui/ImGuiEx.h"
#include "types.h"

namespace ndbl
{
    using tools::Vec2;
    using tools::Vec4;
    using tools::Color;

    struct Config
    {
        Config(tools::Config*);
        void reset();

        TextEditor::Palette ui_text_textEditorPalette{};
        Vec2           ui_wire_bezier_roundness; // {min, max}
        float          ui_wire_bezier_thickness;
        Vec2           ui_wire_bezier_fade_length_minmax;
        Vec4           ui_wire_color;
        Vec4           ui_wire_shadowColor;
        float          ui_slot_radius;
        Vec4           ui_slot_border_color;
        Vec4           ui_slot_color;
        Vec4           ui_slot_hovered_color;
        Vec2           ui_slot_size;
        float          ui_slot_gap;
        float          ui_slot_border_radius;
        float          ui_slot_invisible_ratio;
        float          ui_node_spacing;
        Vec4           ui_node_padding; // left, top, right, bottom
        float          ui_node_borderWidth;
        float          ui_node_instructionBorderRatio; // ratio to apply to borderWidth
        Vec4           ui_node_variableColor;
        Vec4           ui_node_invokableColor;
        Vec4           ui_node_instructionColor;
        Vec4           ui_node_literalColor;
        Vec4           ui_node_condStructColor;
        Vec4           ui_node_shadowColor;
        Vec4           ui_node_borderColor;
        Vec4           ui_node_borderHighlightedColor;
        Vec4           ui_node_highlightedColor;
        Vec4           ui_node_fillColor;
        float          ui_node_speed;
        u8_t           ui_node_animation_subsample_count;
        Vec4           ui_codeflow_color;
        Vec4           ui_codeflow_shadowColor;
        float          ui_codeflow_thickness_ratio;
        Vec2           ui_toolButton_size;
        u64_t          ui_history_size_max{};
        float          ui_history_btn_spacing;
        float          ui_history_btn_height;
        float          ui_history_btn_width_max;
        const char*    ui_splashscreen_imagePath;
        float          ui_overlay_margin;
        float          ui_overlay_indent;
        Vec4           ui_overlay_window_bg_golor;
        Vec4           ui_overlay_border_color;
        Vec4           ui_overlay_text_color;
        Vec4           ui_graph_grid_color_major;
        Vec4           ui_graph_grid_color_minor;
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
        bool           draw_debug_lines;
        tools::Config* tools_cfg;

        // Computed values

        int    ui_grid_subdiv_size() const;
        float  ui_codeflow_thickness() const;
    };

    Config* init_config(); // create a new configuration and set it as current
    void    shutdown_config(); // do the opposite of init
    Config* get_config(); // Get the current config, create_config() must be called first.
}
