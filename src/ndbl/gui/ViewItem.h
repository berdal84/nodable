#pragma once
#include <utility>
#include <variant>
#include <cstddef>

namespace ndbl
{
    // forward declarations
    class SlotView;
    class NodeView;

    enum ViewItemType
    {
        ViewItemType_NULL = 0,
        ViewItemType_SLOT,
        ViewItemType_EDGE,
        ViewItemType_SCOPE,
        ViewItemType_NODE,
    };

    // Simple structure to store a NodeView, a SlotView, an Edge, or nothing.
    struct ViewItem
    {
        ViewItemType type  = ViewItemType_NULL;

        union
        {
            struct {
                void *ptr1;
                void *ptr2;
            } raw_data;

            NodeView*  nodeview;
            SlotView*  slotview;
            ScopeView* scopeview;

            struct {
                SlotView* slot[2];
            } edge;

        };

        ViewItem()
        : raw_data({nullptr, nullptr})
        {}

        ViewItem(SlotView* slotview)
        : type(ViewItemType_SLOT)
        , raw_data({slotview, nullptr})
        {}

        ViewItem(ScopeView* scopeview)
        : type(ViewItemType_SCOPE)
        , raw_data({scopeview, nullptr})
        {}

        ViewItem(NodeView* nodeview)
        : type(ViewItemType_NODE)
        , raw_data({nodeview, nullptr})
        {}

        ViewItem(SlotView* edge_start, SlotView* edge_end)
        : type(ViewItemType_EDGE)
        , raw_data({edge_start, edge_end})
        {}

        bool empty() const
        { return type == ViewItemType_NULL; }
    };
}