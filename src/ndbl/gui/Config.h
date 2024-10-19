#pragma once

#include <string>
#include <vector>
#include <memory>

#include "tools/core/reflection/reflection"
#include "tools/core/types.h"
#include "tools/gui/AppView.h"
#include "tools/gui/Config.h"
#include "tools/gui/FontManager.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/gui/Size.h"

#include "ndbl/core/NodeType.h"

#include "types.h"
#include "ViewDetail.h"
#include "Isolation.h"

namespace ndbl
{
    using tools::Vec2;
    using tools::Vec4;
    using tools::Color;

    typedef int ConfigFlags;
    enum ConfigFlag_
    {
        ConfigFlag_NONE                              = 0,
        ConfigFlag_DRAW_DEBUG_LINES                  = 1 << 0,
        ConfigFlag_EXPERIMENTAL_GRAPH_AUTOCOMPLETION = 1 << 1,
        ConfigFlag_EXPERIMENTAL_HYBRID_HISTORY       = 1 << 2,
        ConfigFlag_EXPERIMENTAL_MULTI_SELECTION      = 1 << 3,
        ConfigFlag_EXPERIMENTAL_INTERPRETER          = 1 << 4,
    };

    struct Config
    {
        Config(tools::Config*);
        void reset();

        TextEditor::Palette ui_text_textEditorPalette{};
        Vec2           ui_wire_bezier_roundness; // {min, max}
        float          ui_wire_bezier_thickness;
        Vec2           ui_wire_bezier_fade_lensqr_range;
        Vec4           ui_wire_color;
        Vec4           ui_wire_shadowColor;
        float          ui_slot_circle_radius_base;
        float          ui_slot_circle_radius(tools::Size = tools::Size_DEFAULT) const;
        Vec4           ui_slot_border_color;
        Vec4           ui_slot_color;
        Vec4           ui_slot_hovered_color;
        Vec2           ui_slot_rectangle_size;
        float          ui_slot_gap;
        float          ui_slot_border_radius;
        float          ui_slot_invisible_ratio;
        Vec2           ui_node_gap_base; // horizontal, vertical
        Vec2           ui_node_gap(tools::Size = tools::Size_DEFAULT) const;
        Vec4           ui_node_padding; // left, top, right, bottom
        float          ui_node_borderWidth;
        float          ui_node_instructionBorderRatio; // ratio to apply to borderWidth
        std::array<Vec4,NodeType_COUNT> ui_node_fill_color;
        Vec4           ui_node_shadowColor;
        Vec4           ui_node_borderColor;
        Vec4           ui_node_borderHighlightedColor;
        Vec4           ui_node_highlightedColor;
        float          ui_node_speed;
        u8_t           ui_node_animation_subsample_count;
        ViewDetail     ui_node_detail;
        Vec4           ui_codeflow_color;
        Vec4           ui_codeflow_shadowColor;
        float          ui_codeflow_thickness_ratio;
        float          ui_codeflow_thickness() const;
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
        const char*    ui_interpreter_window_label;
        Isolation      isolation;
        float          graph_unfold_dt;
        i32_t          graph_unfold_iterations;
        ConfigFlags    flags;
        tools::Config* tools_cfg;

        bool has_flags(ConfigFlags _flags)const { return (flags & _flags) == _flags; };
        void set_flags(ConfigFlags _flags) { flags |= _flags; }
        void clear_flags(ConfigFlags _flags) { flags &= ~_flags; }
    };

    [[nodiscard]]
    Config* init_config(); // create a new configuration and set it as current
    void    shutdown_config(Config*); // do the opposite of init
    Config* get_config(); // Get the current config, create_config() must be called first.
}
