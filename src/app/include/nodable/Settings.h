#pragma once

#include <string>
#include <vector>

#include <imgui/imgui.h>
#include <nodable/Reflect.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <nodable/FontConf.h>
#include <nodable/FontSlot.h>
#include <nodable/types.h>

namespace Nodable {

    // forward decl
    class AppContext;

    class Settings  {
        friend class AppContext;
    public:
        std::vector<FontConf>                    ui_text_fonts;
        std::array<const char *, FontSlot_COUNT> ui_text_defaultFontsId;
        TextEditor::Palette                      ui_text_textEditorPalette;
        FontConf ui_icons;
        float  ui_wire_bezier_roundness;
        float  ui_wire_bezier_thickness;
        bool   ui_wire_displayArrows;
        vec4 ui_wire_fillColor;
        vec4 ui_wire_shadowColor;
        float  ui_node_memberConnectorRadius;
        float  ui_node_padding;
        vec4 ui_node_variableColor;
        vec4 ui_node_invokableColor;
        vec4 ui_node_instructionColor;
        vec4 ui_node_literalColor;
        vec4 ui_node_shadowColor;
        vec4 ui_node_borderHighlightedColor;
        vec4 ui_node_borderColor;
        vec4 ui_node_highlightedColor;
        vec4 ui_node_fillColor;
        vec4 ui_node_nodeConnectorColor;
        vec4 ui_node_nodeConnectorHoveredColor;
        float  ui_node_spacing;
        float  ui_node_speed;
        float  ui_node_connector_height;
        float  ui_node_connector_padding;
        float  ui_layout_propertiesRatio;
        float  ui_node_connector_width;
        vec4 ui_codeFlow_lineColor;
        vec4 ui_codeFlow_lineShadowColor;
        vec4 ui_button_activeColor;
        vec4 ui_button_hoveredColor;
        vec4 ui_button_color;
        vec2 ui_toolButton_size;
        const char* ui_splashscreen_imagePath;
        bool graph_autocompletion;
        void patch_imgui_style(ImGuiStyle&);

    private:
        static Settings* create_default();

        REFLECT(Settings)
    };
}
