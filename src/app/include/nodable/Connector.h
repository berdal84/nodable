#pragma once
#include <nodable/Way.h>
#include <nodable/types.h>

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
        virtual vec2     get_pos()const = 0;
        virtual bool     share_parent_with(const T *) const = 0;

        static void      start_drag(const T* connector) { if(T::s_dragged == nullptr) T::s_dragged = connector; }
        static const T*  get_hovered() { return T::s_hovered; }
        static const T*  get_gragged() { return T::s_dragged; }
        static const T*  get_focused() { return T::s_dragged; }
        static bool      is_dragging() { return T::s_dragged; }
        static void      stop_drag() { T::s_dragged = nullptr; }
        static void      set_focused(const T* connector) { T::s_focused = connector; }
        static void      unset_focused() { T::s_focused = nullptr; }

        static void drop_behavior(bool& require_new_node, bool _enable_edition)
        {
            if (T::s_dragged && ImGui::IsMouseReleased(0))
            {
                if ( _enable_edition )
                {
                    if ( T::s_hovered )
                    {
                        T::dropped(T::s_dragged, T::s_hovered);
                        T::s_dragged = T::s_hovered = nullptr;
                    } else {
                        require_new_node = true;
                    }
                }
                else
                {
                    T::s_dragged = nullptr;
                }
            }
        }
    };
}