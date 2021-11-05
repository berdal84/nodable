#pragma once

#include <string>
#include <vector>
#include <imgui/imgui.h>
#include <ImGuiColorTextEdit/TextEditor.h>
#include <nodable/FontConf.h>
#include "FontSlot.h"

namespace Nodable {

    struct Settings {
    public:
        struct {
            struct {
                std::vector<FontConf> fonts;
                std::array<const char *, FontSlot_COUNT> defaultFontsId;
                TextEditor::Palette textEditorPalette;
            } text;

            FontConf icons;

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

            struct {
                std::string imagePath;
            } splashscreen;
        } ui;

        void setImGuiStyle(ImGuiStyle&);

        /** Instantiate (the first time) and get the current settings */
        static Settings* GetCurrent();
    };
}
