#pragma once

#include <imgui.h>

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
            } text;

            struct {
                struct {
                    float roundness;
                    float thickness;
                } bezier;
                bool displayArrows;
            } wire;

            struct {
                float connectorRadius;
                float padding;
                ImVec4 variableColor;
                ImVec4 functionColor;
                ImVec4 instructionColor;
                ImVec4 shadowColor;
                ImVec4 borderHighlightedColor;
                ImVec4 borderColor;
                ImVec4 highlightedColor;
                ImVec4 fillColor;
            } nodes;

            struct {
                float propertiesRatio;
            } layout;

            struct {
                float lineWidthMax;
            } codeFlow;

        } ui;

        void setImGuiStyle(ImGuiStyle&);

        /** Instantiate (the first time) and get the current settings */
        static Settings* GetCurrent();
    };
}
