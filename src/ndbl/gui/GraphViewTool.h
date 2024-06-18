#pragma once

#include <cstring>
#include <utility>
#include <vector>
#include <variant>
#include <any>
#include "tools/core/geometry/Vec2.h"

namespace ndbl
{
    // Forward declarations
    class NodeView;
    class SlotView;

    enum ItemType
    {
        ItemType_NONE = 0,
        ItemType_NODEVIEW,
        ItemType_SLOTVIEW,
        ItemType_POSITION,
        ItemType_EDGE
    };

    struct NodeView_Item
    {
        ItemType type{ItemType_NODEVIEW};
        NodeView* view;
        NodeView_Item(NodeView* view): view(view) {}
    };

    struct SlotView_Item
    {
        ItemType  type{ItemType_SLOTVIEW};
        SlotView* view;
        SlotView_Item(SlotView* v): view(v) {}
    };

    struct Edge_Item
    {
        ItemType  type{ItemType_EDGE};
        SlotView* tail;
        SlotView* head;
        Edge_Item(SlotView* tail, SlotView* head): tail(tail), head(head) {}
    };

    struct Pixel_Item
    {
        ItemType    type{ItemType_POSITION};
        ImVec2      pos{};
        Pixel_Item() = default;
        Pixel_Item(tools::Vec2  pos): pos(std::move(pos)) {}
    };

    union Item
    {
        ItemType      type;
        NodeView_Item node;
        SlotView_Item slot;
        Edge_Item     edge;
        Pixel_Item    pixel;

        Item() { memset(this, 0, sizeof(Item)); }
        Item(const NodeView_Item& item): Item() { node = item; }
        Item(const SlotView_Item& item): Item() { slot = item; }
        Item(const Edge_Item& item): Item() { edge = item; }
        Item(const Pixel_Item& item): Item() { pixel = item; }
    };

    enum ToolType
    {
        ToolType_NONE = 0,
        ToolType_DEFINE_ROI,
        ToolType_DRAG,
        ToolType_CREATE_WIRE
    };

    struct ROI_Tool
    {
        ToolType type{ToolType_DEFINE_ROI};
        ImVec2   start_pos;
        ImVec2   end_pos;

        ROI_Tool(const tools::Vec2& pos): start_pos(pos), end_pos(pos) {}

        tools::Rect get_rect() const
        {
            return tools::Rect::normalize({this->start_pos, this->end_pos });
        }

    };

    struct DragNodeViews_Tool
    {
        enum class Mode
        {
            SELECTION = 0,
            ALL = 1,
        };

        ToolType type{ToolType_DRAG};
        Mode     mode;
        DragNodeViews_Tool(Mode mode = Mode::ALL): mode(mode) {}
    };

    struct DrawWire_Tool
    {
        ToolType       type{ToolType_CREATE_WIRE};
        SlotView_Item dragged_slot;
        DrawWire_Tool(const SlotView_Item& from): dragged_slot(from) {}
    };

    union Tool
    {
        ToolType           type;
        ROI_Tool           roi;
        DragNodeViews_Tool drag;
        DrawWire_Tool      wire;

        Tool() { memset(this, 0, sizeof(Tool)); }

        Tool(const ROI_Tool& t): Tool() { roi = t; }
        Tool(const DragNodeViews_Tool& t): Tool() { drag = t; }
        Tool(const DrawWire_Tool& t): Tool() { wire = t; }
    };
}