#pragma once

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
            } nodes;

            struct {
                float propertiesRatio;
            } layout;
        } ui;

        /** Get the current configuration */
        static Settings& GetCurrent();
    };
}
