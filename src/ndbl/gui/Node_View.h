#pragma once

#include <string>
#include <unordered_map>

#include "tools/core/Component.h"// base class
#include "tools/gui/geometry/Box_2D.h"
#include "tools/gui/View_State.h"
#include "ndbl/core/Node_Property.h"
#include "Node_Property_View.h"
#include "Node_Slot_View.h"

namespace ndbl
{
    // forward declaration
    class Node;
    class Graph;
    class Scope_View;
    struct Node_Slot;
    struct Node_Slot_View;
    class Graph_View;

    /**
     * Enum to define some color types
     */
    enum ColorType
    {
        Color_FILL,
        Color_COUNT
    };

    typedef int Node_ViewFlags;
    enum Node_ViewFlag_
    {
        // note: when adding a new value, remember we want NONE to be the most common case

        Node_ViewFlag_NONE                   = 0,
        Node_ViewFlag_WITH_RECURSION         = 1 << 0,
        Node_ViewFlag_WITH_PINNED            = 1 << 1,
        Node_ViewFlag_EXCLUDE_UNSELECTED     = 1 << 2
    };

	/**
	 * This class implement a view for Nodes using ImGui.
	 */
    class Node_View : public tools::Component<Node>
	{
        constexpr static tools::Vec4 DEFAULT_COLOR{1.f, 0.f, 0.f};
//== Data ==============================================================================================================
    private:
        enum PropType
        {
            PropType_IN_STRICTLY = 0,
            PropType_OUT_STRICTLY,
            PropType_INOUT_STRICTLY,
            PropType_IN,
            PropType_OUT,
            PropType_COUNT
        };

        tools::Box_2D               m_shape;
        tools::View_State           m_view_state;
        bool                        m_expanded{true};
        float                       m_opacity{1.f};
        Node_Slot_View*             m_hovered_slotview{};
        Scope_View*                 m_internal_scopeview{};
        std::array<const tools::Vec4*, Color_COUNT> m_colors {&DEFAULT_COLOR};
        std::vector<Node_Slot_View*>     m_slot_views;
        std::unordered_map<const Node_Property*, Node_Property_View*> m_view_by_property;
        Node_Property_View*              m_value_view{};
        std::array<std::vector<Node_Property_View*>, PropType::PropType_COUNT> m_view_by_property_type;
//== Methods ===========================================================================================================
    public:
        friend class Graph_View;
        Node_View();
		~Node_View() override;

        Node*                   node() const { return entity; } // entity() alias
        std::vector<Node_View*> get_adjacent(Node_Slot::Flags) const;
        bool                    draw();
        void                    update(float);
        void                    arrange_recursively(bool smoothly = true);
        std::string             get_label();
        tools::Rect             get_rect(tools::Space space = tools::WORLD_SPACE) const { return m_shape.rect(space); }
        tools::Rect             get_rect_ex(tools::Space, Node_ViewFlags) const;
        bool                    expanded()const { return m_expanded; }
        void                    set_expanded_rec(bool);
        void                    set_expanded(bool);
        void                    set_inputs_visible(bool visible, bool recursive = false);
        void                    set_children_visible(bool visible, bool recursive = false);
        void                    expand_toggle() { set_expanded(!m_expanded); }
        void                    expand_toggle_rec() { return set_expanded_rec(!m_expanded); };
        void                    set_color( const tools::Vec4*, ColorType = Color_FILL );
        tools::Vec4             get_color(ColorType) const;
        void                    set_size(const tools::Vec2& size) { m_shape.set_size(size); }
        tools::Box_2D*          shape() { return &m_shape; }
        const tools::Box_2D*    shape() const { return &m_shape; }
        void                    translate(const tools::Vec2& delta) { m_shape.spatial_node()->translate(delta); }
        const tools::Spatial_Node* spatial_node() const { return m_shape.spatial_node(); }
        tools::Spatial_Node*     spatial_node() { return m_shape.spatial_node(); }
        tools::View_State*       state() { return &m_view_state; }
        const tools::View_State* state() const { return &m_view_state; }
        void                    reset_all_properties();
        Scope_View*             internal_scopeview() { return m_internal_scopeview; }
        const Scope_View*       internal_scopeview() const { return m_internal_scopeview; }
        static tools::Rect      bounding_rect(const std::vector<Node_View *>&, tools::Space = tools::WORLD_SPACE, Node_ViewFlags = Node_ViewFlag_NONE);
        static bool             draw_as_properties_panel(Node_View*, bool* show_advanced );
        static Node_View*       substitute_with_parent_if_not_visible(Node_View*, bool _recursive = true);

    private:
        void                    _handle_init();
        void                    _handle_shutdown();
        Node_Property_View*     _find_property_view(const Node_Property*);
        void                    _draw_slot(Node_Slot_View*);
        void                    _set_adjacent_visible(Node_Slot::Flags, bool _visible, Node_ViewFlags = Node_ViewFlag_NONE);

        static void DrawNodeRect(
                tools::Rect rect,
                tools::Vec4 color,
                tools::Vec4 border_highlight_col,
                tools::Vec4 shadow_col,
                tools::Vec4 border_col,
            bool selected,
            float border_radius,
            float border_width
        );
    };
}
