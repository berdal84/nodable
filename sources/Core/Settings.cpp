#include "Settings.h"

Nodable::Settings &Nodable::Settings::GetCurrent()
{
    static Settings* g_conf = nullptr;

    if ( g_conf == nullptr)
    {
        g_conf = new Settings();

        // TODO: create themes

        // nodes
        g_conf->ui.nodes.padding                = 6.0f;
        g_conf->ui.nodes.connectorRadius        = 5.0f;
        // wires
        g_conf->ui.wire.bezier.roundnessIn      = 0.1f;
        g_conf->ui.wire.bezier.roundnessOut     = 0.1f;
        g_conf->ui.wire.bezier.thickness        = 2.0f;
        g_conf->ui.wire.displayArrows           = false;
    }
    return *g_conf;
};