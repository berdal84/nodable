#pragma once
#include <imgui/imgui.h>
#include <nodable/Way.h>

namespace Nodable {

    // forward declarations
    class Node;

    /**
     * @brief Base abstract class for all connectors
     */
    template<typename T>
    class Connector
    {
    public:
        virtual ImVec2   getPos()const = 0;
        virtual bool     hasSameParentWith(const T *) const = 0;
        virtual bool     connect(const T*) const = 0;

        static void      StartDrag(const T* connector) { if( T::s_dragged == nullptr) T::s_dragged = connector; }
        static const T*  GetHovered() { return T::s_hovered; }
        static const T*  GetDragged() { return T::s_dragged; }
        static const T*  GetFocused() { return T::s_dragged; }
        static bool      IsDragging() { return T::s_dragged; }
        static void      StopDrag() { T::s_dragged = nullptr; }
        static void      SetFocused(const T* connector) { T::s_focused = connector; }
        static void      UnsetFocused() { T::s_focused = nullptr; }
    };
}