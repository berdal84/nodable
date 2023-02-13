#pragma once

#include <string>
#include <vector>
#include <memory>

#include <fw/gui/types.h>
#include <fw/gui/FontConf.h>
#include <fw/gui/FontSlot.h>
#include <fw/gui/AppView.h>
#include <fw/core/reflection/reflection>

#include <ImGuiColorTextEdit/TextEditor.h>
#include <ndbl/gui/types.h>

namespace ndbl {

    class Settings  {
    public:

        Settings();

        TextEditor::Palette ui_text_textEditorPalette;
        float          ui_wire_bezier_roundness;
        float          ui_wire_bezier_thickness;
        bool           ui_wire_displayArrows;
        fw::ImVec4 ui_wire_fillColor;
        fw::ImVec4 ui_wire_shadowColor;
        float          ui_node_propertyConnectorRadius;
        float          ui_node_padding;
        fw::ImVec4 ui_node_variableColor;
        fw::ImVec4 ui_node_invokableColor;
        fw::ImVec4 ui_node_instructionColor;
        fw::ImVec4 ui_node_literalColor;
        fw::ImVec4 ui_node_shadowColor;
        fw::ImVec4 ui_node_borderHighlightedColor;
        fw::ImVec4 ui_node_borderColor;
        fw::ImVec4 ui_node_highlightedColor;
        fw::ImVec4 ui_node_fillColor;
        fw::ImVec4 ui_node_nodeConnectorColor;
        fw::ImVec4 ui_node_nodeConnectorHoveredColor;
        float          ui_node_spacing;
        float          ui_node_speed;
        u8_t           ui_node_animation_subsample_count;
        float          ui_node_connector_height;
        float          ui_node_connector_padding;
        float          ui_node_connector_width;
        fw::ImVec4 ui_codeFlow_lineColor;
        fw::ImVec4 ui_codeFlow_lineShadowColor;
        fw::ImVec2 ui_toolButton_size;
        float          ui_history_btn_spacing;
        float          ui_history_btn_height;
        float          ui_history_btn_width_max;
        const char*    ui_splashscreen_imagePath;
        float          ui_overlay_margin;
        float          ui_overlay_indent;
        fw::ImVec4 ui_overlay_window_bg_golor;
        fw::ImVec4 ui_overlay_border_color;
        fw::ImVec4 ui_overlay_text_color;
        const char*    ui_file_info_window_label;
        const char*    ui_help_window_label;
        const char*    ui_imgui_settings_window_label;
        const char*    ui_node_properties_window_label;
        const char*    ui_settings_window_label;
        const char*    ui_startup_window_label;
        const char*    ui_toolbar_window_label ;
        const char*    ui_virtual_machine_window_label;
        bool           experimental_graph_autocompletion;
        bool           experimental_hybrid_history;
        bool           isolate_selection;
        fw::AppView::Conf fw_app_view;

        static Settings& get_instance();                  // Get the shared settings
    };
}
