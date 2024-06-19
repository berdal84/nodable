#pragma once

#include <cstring>
#include <utility>
#include "tools/core/geometry/Vec2.h"
#include "tools/core/geometry/Rect.h"
#include "CreateNodeCtxMenu.h"

class ImDrawList;

namespace ndbl
{
    // Forward declarations
    class NodeView;
    class SlotView;
    class GraphView;

    enum ItemType
    {
        ItemType_NONE = 0,
        ItemType_NODEVIEW,
        ItemType_SLOTVIEW,
        ItemType_EDGE
    };

    struct NodeViewItem
    {
        ItemType type{ItemType_NODEVIEW};
        NodeView* view;
        NodeViewItem(NodeView* view): view(view) {}
    };

    struct SlotViewItem
    {
        ItemType  type{ItemType_SLOTVIEW};
        SlotView* view;
        SlotViewItem(SlotView* v): view(v) {}
    };

    struct EdgeItem
    {
        ItemType  type{ItemType_EDGE};
        SlotView* tail;
        SlotView* head;
        EdgeItem(SlotView* tail, SlotView* head): tail(tail), head(head) {}
    };


    union Item
    {
        ItemType     type;
        NodeViewItem node;
        SlotViewItem slot;
        EdgeItem     edge;

        Item() { memset(this, 0, sizeof(Item)); }
        Item(const NodeViewItem& item): Item() { node = item; }
        Item(const SlotViewItem& item): Item() { slot = item; }
        Item(const EdgeItem& item): Item() { edge = item; }
    };

    enum ToolType
    {
        ToolType_CURSOR = 0,
        ToolType_ROI,
        ToolType_DRAG,
        ToolType_LINE
    };

    struct ROITool
    {
        ToolType    type{ToolType_ROI};
        tools::Vec2 start_pos;
        tools::Vec2 end_pos;

        ROITool(const tools::Vec2& pos): start_pos(pos), end_pos(pos) {}

        tools::Rect get_rect() const
        {
            return tools::Rect::normalize({this->start_pos, this->end_pos });
        }

    };

    struct DragTool
    {
        enum class Mode
        {
            SELECTION = 0,
            ALL = 1,
        };

        ToolType type{ToolType_DRAG};
        Mode     mode;
        DragTool(Mode mode = Mode::ALL): mode(mode) {}
    };

    struct LineTool
    {
        ToolType      type{ToolType_LINE};
        SlotViewItem dragged_slot;
        LineTool(const SlotViewItem& from): dragged_slot(from) {}
    };

    class Tool
    {
    public:
        // TODO: consider object oriented approach if it gets too complex

        union State
        {
            ToolType  type;
            ROITool   roi;
            DragTool  drag;
            LineTool  wire;
            State() { memset(this, 0, sizeof(State)); }
        };

        struct Context
        {
            GraphView*  graph_view{nullptr};
            tools::Vec2 mouse_pos{};
            tools::Vec2 mouse_pos_snapped{};
            ImDrawList* draw_list{nullptr};
            Item        hovered{};
            Item        focused{};
            bool        cxt_menu_open_last_frame{false};
            CreateNodeCtxMenu create_node_ctx_menu;
            std::vector<NodeView*> selected_nodeview;
        };

        Tool(Context& ctx): context(ctx) { }

        ToolType tool_type() const;
        void     tick();
        void     draw();
    private:
        void     reset_state();
        void     change_state(const State& new_state);
        void     check_state();
        Context& context;
        State    state{};
    };
}