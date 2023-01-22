#pragma once

#include <string>
#include <vector>
#include <memory>

#include <fw/imgui/types.h>
#include <fw/imgui/FontConf.h>
#include <fw/imgui/FontSlot.h>
#include <fw/reflection/reflection>

#include <imgui/imgui.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <nodable/app/types.h>

namespace ndbl {

    class Settings  {
    public:

        Settings();

        size_t                                   ui_log_tooltip_max_count;
        std::array<fw::vec4, fw::Log::Verbosity_COUNT>   ui_log_color;
        std::vector<FontConf>                    ui_text_fonts;
        std::array<const char *, FontSlot_COUNT> ui_text_defaultFontsId;
        TextEditor::Palette                      ui_text_textEditorPalette;
        FontConf       ui_icons;
        float          ui_wire_bezier_roundness;
        float          ui_wire_bezier_thickness;
        bool           ui_wire_displayArrows;
        fw::vec4       ui_wire_fillColor;
        fw::vec4       ui_wire_shadowColor;
        float          ui_node_propertyConnectorRadius;
        float          ui_node_padding;
        fw::vec4       ui_node_variableColor;
        fw::vec4       ui_node_invokableColor;
        fw::vec4       ui_node_instructionColor;
        fw::vec4       ui_node_literalColor;
        fw::vec4       ui_node_shadowColor;
        fw::vec4       ui_node_borderHighlightedColor;
        fw::vec4       ui_node_borderColor;
        fw::vec4       ui_node_highlightedColor;
        fw::vec4       ui_node_fillColor;
        fw::vec4       ui_node_nodeConnectorColor;
        fw::vec4       ui_node_nodeConnectorHoveredColor;
        float          ui_node_spacing;
        float          ui_node_speed;
        u8_t           ui_node_animation_subsample_count;
        float          ui_node_connector_height;
        float          ui_node_connector_padding;
        float          ui_dockspace_right_ratio;
        float          ui_dockspace_down_size;
        float          ui_node_connector_width;
        fw::vec4       ui_codeFlow_lineColor;
        fw::vec4       ui_codeFlow_lineShadowColor;
        fw::vec4       ui_button_activeColor;
        fw::vec4       ui_button_hoveredColor;
        fw::vec4       ui_button_color;
        fw::vec2       ui_toolButton_size;
        float          ui_history_btn_spacing;
        float          ui_history_btn_height;
        float          ui_history_btn_width_max;
        const char*    ui_splashscreen_imagePath;
        float          ui_overlay_margin;
        float          ui_overlay_indent;
        fw::vec4       ui_overlay_window_bg_golor;
        fw::vec4       ui_overlay_border_color;
        fw::vec4       ui_overlay_text_color;
        bool           experimental_graph_autocompletion;
        bool           experimental_hybrid_history;
        bool           isolate_selection;

        void             patch_imgui_style(ImGuiStyle&);  // Apply the settings to an existing ImGuiStyle
        static Settings& get_instance();                  // Get the shared settings
    };
}
