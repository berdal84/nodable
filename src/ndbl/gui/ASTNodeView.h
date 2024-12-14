#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "tools/core/ComponentFor.h"// base class
#include "tools/gui/geometry/BoxShape2D.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/gui/ViewState.h"
#include "ndbl/core/ASTNodeProperty.h"
#include "ndbl/gui/concepts/CView.h"
#include "ASTNodePropertyView.h"
#include "ASTNodeSlotView.h"
#include "types.h"
#include "ViewDetail.h"

namespace ndbl
{
    // forward declaration
    class ASTNode;
    class Graph;
    class ASTScopeView;
    struct ASTNodeSlot;
    struct ASTNodeSlotView;
    class GraphView;

    /**
     * Enum to define some color types
     */
    enum ColorType
    {
        Color_FILL,
        Color_COUNT
    };

    typedef int NodeViewFlags;
    enum NodeViewFlag_
    {
        // note: when adding a new value, remember we want NONE to be the most common case

        NodeViewFlag_NONE                   = 0,
        NodeViewFlag_WITH_RECURSION         = 1 << 0,
        NodeViewFlag_WITH_PINNED            = 1 << 1,
        NodeViewFlag_WITH_MULTICONSTRAINED  = 1 << 2,
        NodeViewFlag_EXCLUDE_UNSELECTED     = 1 << 3
    };

	/**
	 * This class implement a view for Nodes using ImGui.
	 */
    class ASTNodeView : public tools::ComponentFor<ASTNode>
	{
    public:
        DECLARE_REFLECT_override
        friend class GraphView;
        ASTNodeView() = delete;
		ASTNodeView(tools::BoxShape2D*);
		~ASTNodeView() override;

        ASTNode*                node() const { return m_node; }
        std::vector<ASTNodeView*>  get_adjacent(SlotFlags) const;
        bool                    draw();
        void                    update(float);
        void                    arrange_recursively(bool _smoothly = true);
        std::string             get_label();
        tools::Rect             get_rect(tools::Space space = tools::WORLD_SPACE) const;
        tools::Rect             get_rect_ex(tools::Space, NodeViewFlags) const;
        bool                    expanded()const { return m_expanded; }
        void                    set_expanded_rec(bool _expanded);
        void                    set_expanded(bool _expanded);
        void                    set_inputs_visible(bool _visible, bool _recursive = false);
        void                    set_children_visible(bool visible, bool recursively = false);
        void                    expand_toggle() { set_expanded(!m_expanded); }
        void                    expand_toggle_rec() { return set_expanded_rec(!m_expanded); };
        void                    set_color( const tools::Vec4* _color, ColorType _type = Color_FILL );
        tools::Vec4             get_color(ColorType _type) const;
        tools::BoxShape2D*      shape() { return m_shape; }
        const tools::BoxShape2D*shape() const { return m_shape; }
        void                    translate(const tools::Vec2& delta) { m_spatial_node->translate(delta); }
        const tools::SpatialNode* spatial_node() const { return m_spatial_node; }
        tools::SpatialNode*     spatial_node() { return m_spatial_node; }
        tools::ViewState*       state() { return &m_view_state; }
        const tools::ViewState* state() const { return &m_view_state; }
        void                    reset_all_properties();

        static tools::Rect      get_rect(const std::vector<ASTNodeView *> &_views, tools::Space = tools::WORLD_SPACE, NodeViewFlags = NodeViewFlag_NONE);
        static std::vector<tools::Rect>   get_rects(const std::vector<ASTNodeView*>& _in_views, tools::Space space = tools::WORLD_SPACE, NodeViewFlags flags = NodeViewFlag_NONE);
        static bool             is_inside(ASTNodeView*, const tools::Rect&, tools::Space = tools::WORLD_SPACE);
        static void             constraint_to_rect(ASTNodeView*, const tools::Rect& );
        static bool             draw_as_properties_panel(ASTNodeView* _view, bool* _show_advanced );
        static ASTNodeView*        substitute_with_parent_if_not_visible(ASTNodeView* _view, bool _recursive = true);
        static std::vector<ASTNodeView*> substitute_with_parent_if_not_visible(const std::vector<ASTNodeView*>& _in, bool _recurse = true );

    private:
        void                    on_owner_init(ASTNode*);
        ASTNodePropertyView*    find_property_view(const ASTNodeProperty *pProperty);
        void                    add_child(CView auto* view);
        void                    draw_slot(ASTNodeSlotView*);
        void                    set_adjacent_visible(SlotFlags, bool _visible, NodeViewFlags = NodeViewFlag_NONE);

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

        ASTNode*                   m_node;
        tools::SpatialNode*        m_spatial_node;
        tools::BoxShape2D*         m_shape;
        tools::ViewState           m_view_state;
        bool                       m_expanded;
        float                      m_opacity;
        ASTNodeSlotView*       m_hovered_slotview;
        ASTNodeSlotView*       m_last_clicked_slotview;
        std::array<const tools::Vec4*, Color_COUNT> m_colors;
        std::vector<ASTNodeSlotView*>     m_slot_views;
        std::unordered_map<const ASTNodeProperty*, ASTNodePropertyView*> m_view_by_property;
        ASTNodePropertyView*              m_value_view;

        enum PropType
        {
            PropType_IN_STRICTLY = 0,
            PropType_OUT_STRICTLY,
            PropType_INOUT_STRICTLY,
            PropType_IN,
            PropType_OUT,
            PropType_COUNT
        };

        std::array<std::vector<ASTNodePropertyView*>, PropType::PropType_COUNT> m_view_by_property_type;
    };

    void ASTNodeView::add_child(CView auto* child)
    {
        spatial_node()->add_child( child->spatial_node() );
        child->spatial_node()->set_position({0.f, 0.f}, tools::PARENT_SPACE);

        if constexpr ( std::is_same_v<decltype(child), ASTNodeSlotView*> )
        {
            m_slot_views.push_back(child);
        }
    }
}
