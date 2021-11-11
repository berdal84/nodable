#pragma once

#include <string>
#include <vector>
#include <imgui/imgui.h>
#include <mirror.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <nodable/FontConf.h>
#include <nodable/FontSlot.h>

namespace Nodable {

    struct Settings {
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
        ImVec4 ui_node_functionColor;
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
        float  ui_node_nodeConnectorHeight;
        float  ui_node_nodeConnectorPadding;
        float  ui_layout_propertiesRatio;
        float  ui_codeFlow_lineWidthMax;
        ImVec4 ui_codeFlow_lineColor;
        ImVec4 ui_codeFlow_lineShadowColor;
        ImVec4 ui_button_activeColor;
        ImVec4 ui_button_hoveredColor;
        ImVec4 ui_button_color;
        std::string ui_splashscreen_imagePath;

        void setImGuiStyle(ImGuiStyle&);

        /** Get the current settings */
        static Settings* Get();

        /** Load from a file */
        static Settings* Load(const char *_each_member);

        /** Log current settings */
        void Log();

        MIRROR_CLASS_NOVIRTUAL(Settings)(
             MIRROR_MEMBER(ui_codeFlow_lineWidthMax)()
             MIRROR_MEMBER(ui_wire_displayArrows)()
        );
    };
}
