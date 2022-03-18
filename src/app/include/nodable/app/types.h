#pragma once

#include <imgui.h>

namespace Nodable
{
    /**
     *  Types only relative to the application (mainly related to View)
     */

    /*
     * Simple alias for vector2 and vector4, TODO: use glm and add extra copy constructor in ImGui config
     */
    typedef ImVec2 vec2;
    typedef ImVec4 vec4;
}