#pragma once

#include <imgui/imgui.h>
#include <ImGuiColorTextEdit/TextEditor.h>

namespace Nodable {

    struct TextStyle {
        float size;
        const char* font;
    };

    struct Settings {
    public:
        struct {
            struct {
                TextStyle p;
                TextStyle h1;
                TextEditor::Palette textEditorPalette;
            } text;

            struct {
                struct {
                    float roundness;
                    float thickness;
                } bezier;
                bool displayArrows;
                ImVec4 fillColor;
                ImVec4 shadowColor;
            } wire;

            struct {
                float memberConnectorRadius;
                float padding;
                ImVec4 variableColor;
                ImVec4 functionColor;
                ImVec4 instructionColor;
                ImVec4 literalColor;
                ImVec4 shadowColor;
                ImVec4 borderHighlightedColor;
                ImVec4 borderColor;
                ImVec4 highlightedColor;
                ImVec4 fillColor;
                ImVec4 nodeConnectorColor;
                ImVec4 nodeConnectorHoveredColor;
                float spacing;
                float speed;
                float nodeConnectorHeight;
                float nodeConnectorPadding;
            } node;

            struct {
                float propertiesRatio;
            } layout;

            struct {
                float  lineWidthMax;
                ImVec4 lineColor;
                ImVec4 lineShadowColor;
            } codeFlow;

            struct {
                ImVec4 activeColor;
                ImVec4 hoveredColor;
                ImVec4 color;
            } button;
        } ui;

        void setImGuiStyle(ImGuiStyle&);

        /** Instantiate (the first time) and get the current settings */
        static Settings* GetCurrent();
    };
}
