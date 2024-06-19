#pragma once

#include <map>
#include <string>
#include <functional>

#include "tools/gui/View.h"  // base class
#include "tools/core/reflection/reflection"

#include "ndbl/core/NodeComponent.h"  // base class
#include "ndbl/core/IScope.h"
#include "ndbl/core/Scope.h"

#include "Action.h"

#include "NodeView.h"
#include "NodeViewConstraint.h"
#include "SlotView.h"
#include "types.h"
#include "GraphViewTool.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;
    using tools::Vec2;

    constexpr const char* k_CONTEXT_MENU_POPUP = "GraphView.ContextMenuPopup";

    typedef int SelectionMode;
    enum SelectionMode_
    {
        SelectionMode_ADD     = 0,
        SelectionMode_REPLACE = 1,
    };

    class GraphView: public tools::View
    {
        REFLECT_DERIVED_CLASS()

    public:
        using NodeViewVec = std::vector<NodeView*>;

	    explicit GraphView(Graph* graph);
		~GraphView() override = default;

        bool        draw() override;
        void        add_action_to_context_menu( Action_CreateNode* _action);
        void        frame_nodes(FrameMode mode );
        bool        selection_empty() const;
        void        reset(); // unfold and frame the whole graph
        bool        update();
        bool        has_an_active_tool() const;
        void        set_selected(const NodeViewVec&, SelectionMode = SelectionMode_REPLACE);
        const NodeViewVec& get_selected() const;
        void        reset_all_properties();
        std::vector<NodeView*> get_all_nodeviews() const;
        void        draw_wire_from_slot_to_pos(SlotView *from, const Vec2 &end_pos);
        Graph*      get_graph() const;

    private:
        void        unfold(); // unfold the graph until it is stabilized
        bool        update(float dt);
        bool        update(float dt, u16_t samples);
        static void translate_all(const std::vector<NodeView*>&, const Vec2& offset, NodeViewFlags);
        void        translate_all(const Vec2& offset);
        bool        is_selected(NodeView*) const;
        void        frame_views(const std::vector<NodeView*>&, bool _align_top_left_corner);

        Graph*        m_graph;
        Tool::Context m_context{};
        Tool          m_tool{m_context};
    };
}