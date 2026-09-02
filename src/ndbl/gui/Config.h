#pragma once

#include "gui/Action.h"
#include "gui/Layout.h"
#include "gui/geometry/Rect.h"
#include "bdc/Types.hpp"
#include "tools/gui/Config.h"
#include "tools/gui/Size.h"
#include "ImGuiColorTextEdit/TextEditor.h"

#include "ndbl/core/Node.h"
#include "ndbl/core/Node_Slot.h"

namespace ndbl
{
    using tools::Vec2;
    using tools::Vec4;
    using tools::Color;

    typedef u8_t Config_Flags;
    enum Config_Flag_ : u8_t
    {
        Config_Flag_NONE                              = 0,
        Config_Flag_ISOLATION_ON                      = 1 << 0,
        Config_Flag_EXPERIMENTAL_HYBRID_COMMAND_MANAGER       = 1 << 1,
        Config_Flag_EXPERIMENTAL_MULTI_SELECTION      = 1 << 2
    };

    typedef u8_t View_Detail;
    enum View_Detail_: u8_t
    {
        View_Detail_NORMAL  = 0,
        View_Detail_COMPACT
    };

    struct Config
    {
        Config_Flags   flags;
        View_Detail    ui_node_detail;
        float          graph_view_unfold_duration; // The virtual duration used to simulate a graph view unfolding, like accelerating time.
        float          ui_codeflow_thickness_ratio;
        float          ui_history_btn_height;
        float          ui_history_btn_spacing;
        float          ui_history_btn_width_max;
        float          ui_node_border_radius;
        float          ui_node_borderWidth;
        float          ui_node_instructionBorderRatio; // ratio to apply to borderWidth
        float          ui_node_physics_frequency;
        float          ui_node_selected_rectangle_offset;
        float          ui_node_speed;
        float          ui_overlay_indent;
        float          ui_scope_border_radius;
        float          ui_scope_border_thickness;
        float          ui_scope_gap_base;
        float          ui_slot_border_radius;
        float          ui_slot_circle_radius_base;
        float          ui_slot_gap;
        float          ui_slot_invisible_btn_expand_size;
        float          ui_textview_padding;
        float          ui_wire_bezier_thickness;
        i32_t          ui_grid_size;
        i32_t          ui_grid_subdiv_count;
        bdc::String    ui_config_window_label;
        bdc::String    ui_file_info_window_label;
        bdc::String    ui_help_window_label;
        bdc::String    ui_imgui_config_window_label;
        bdc::String    ui_interpreter_window_label;
        bdc::String    ui_node_properties_window_label;
        bdc::String    ui_splashscreen_imagePath;
        bdc::String    ui_startup_window_label;
        bdc::String    ui_toolbar_window_label ;
        tools::Config* tools_cfg;
        std::array<Vec4,Node_Type_COUNT> ui_node_fill_color;
        std::vector<tools::Action> actions;
        TextEditor::Palette ui_text_textEditorPalette{};
        tools::Padding ui_scope_padding;
        u64_t          ui_history_size_max{};
        Vec2           ui_node_gap_base; // horizontal, vertical
        Vec2           ui_slot_rectangle_size;
        Vec2           ui_toolButton_size;
        Vec2           ui_wire_bezier_fade_lensqr_range;
        Vec2           ui_wire_bezier_roundness; // {min, max}
        Vec4           ui_codeflow_color;
        Vec4           ui_codeflow_shadowColor;
        Vec4           ui_graph_grid_color_major;
        Vec4           ui_graph_grid_color_minor;
        Vec4           ui_node_borderColor;
        Vec4           ui_node_borderHighlightedColor;
        Vec4           ui_node_highlightedColor;
        Vec4           ui_node_padding; // left, top, right, bottom
        Vec4           ui_node_shadowColor;
        Vec4           ui_overlay_border_color;
        Vec4           ui_overlay_text_color;
        Vec4           ui_overlay_window_bg_golor;
        Vec4           ui_scope_border_col;
        Vec4           ui_scope_fill_col_dark;
        Vec4           ui_scope_fill_col_light;
        Vec4           ui_slot_border_color;
        Vec4           ui_slot_color_dark;
        Vec4           ui_slot_color_light;
        Vec4           ui_slot_hovered_color;
        Vec4           ui_wire_color;
        Vec4           ui_wire_shadowColor;
        
        // computed fields
        
        float          ui_codeflow_thickness() const;
        float          ui_scope_gap(tools::Size size = tools::Size_DEFAULT) const;
        float          ui_slot_circle_radius(tools::Size = tools::Size_DEFAULT) const;
        Vec2           ui_node_gap(tools::Size = tools::Size_DEFAULT) const;
        Vec4&          ui_slot_color(Node_Slot::Flags slot_flags);
    };

    Config* config_init();
    void    config_shutdown(); // do the opposite of init
    void    config_reset();
    Config* config();
}
