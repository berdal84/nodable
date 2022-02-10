#pragma once

#include <string>
#include <vector>

#include <imgui/imgui.h>
#include <nodable/Reflect.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <nodable/FontConf.h>
#include <nodable/FontSlot.h>

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
        ImVec4 ui_wire_fillColor;
        ImVec4 ui_wire_shadowColor;
        float  ui_node_memberConnectorRadius;
        float  ui_node_padding;
        ImVec4 ui_node_variableColor;
        ImVec4 ui_node_invokableColor;
        ImVec4 ui_node_instructionColor;
        ImVec4 ui_node_literalColor;
        ImVec4 ui_node_shadowColor;
        ImVec4 ui_node_borderHighlightedColor;
        ImVec4 ui_node_borderColor;
        ImVec4 ui_node_highlightedColor;
        ImVec4 ui_node_fillColor;
        ImVec4 ui_node_nodeConnectorColor;
        ImVec4 ui_node_nodeConnectorHoveredColor;
        float  ui_node_spacing;
        float  ui_node_speed;
        float  ui_node_connector_height;
        float  ui_node_connector_padding;
        float  ui_layout_propertiesRatio;
        float  ui_node_connector_width;
        ImVec4 ui_codeFlow_lineColor;
        ImVec4 ui_codeFlow_lineShadowColor;
        ImVec4 ui_button_activeColor;
        ImVec4 ui_button_hoveredColor;
        ImVec4 ui_button_color;
        ImVec2 ui_toolButton_size;
        const char* ui_splashscreen_imagePath;

        void patch_imgui_style(ImGuiStyle&);

    private:
        static Settings* create_default();

        REFLECT(Settings)
    };
}
