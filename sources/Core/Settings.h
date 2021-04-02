#pragma once

namespace Nodable {

    struct Settings {
    public:
        struct {
            struct {
                struct {
                    float roundnessIn;
                    float roundnessOut;
                    float thickness;
                } bezier;
                bool displayArrows;
            } wire;

            struct {
                float connectorRadius;
                float padding;
            } nodes;
        } ui;

        /** Get the current configuration */
        static Settings& GetCurrent();
    };
}
