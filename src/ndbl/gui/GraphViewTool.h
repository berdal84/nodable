#pragma once

#include <cstring>
#include "tools/core/geometry/Vec2.h"
#include "tools/core/geometry/Rect.h"
#include "CreateNodeCtxMenu.h"
#include "tools/core/StateMachine.h"

class ImDrawList;

namespace ndbl
{
    // Forward declarations
    class NodeView;
    class SlotView;
    class GraphView;
    struct GraphViewToolContext;

    enum ToolType
    {
        ToolType_CURSOR,
        ToolType_ROI,
        ToolType_DRAG,
        ToolType_LINE,
    };

    // GraphViewTool is a state that can be drawn.
    class GraphViewTool : public tools::State
    {
    public:
        GraphViewTool(ToolType, GraphViewToolContext&);
        virtual void draw() {};
        GraphViewToolContext& m_context;
    };

    class CursorTool : public GraphViewTool
    {
    public:
        CursorTool(GraphViewToolContext&);
        void on_tick() override;
        void draw() override;
    };

    class ROITool : public GraphViewTool
    {
    public:
        ROITool(GraphViewToolContext&);
        void on_enter() override;
        void on_tick() override;
        void draw() override;
        void on_leave() override;
        tools::Rect get_rect() const;
    private:
        tools::Vec2 m_start_pos;
        tools::Vec2 m_end_pos;
    };

    class DragTool : public GraphViewTool
    {
    public:
        enum class Mode { SELECTION = 0, ALL = 1, };

        DragTool(GraphViewToolContext&, Mode mode = Mode::ALL);
        void on_enter() override;
        void on_tick() override;
    private:
        Mode m_mode;
    };

    class LineTool : public GraphViewTool
    {
    public:
        LineTool(GraphViewToolContext&);
        void on_tick() override;
        void draw() override;
    private:
        SlotView* m_dragged_slotview{};
    };

    class GraphViewToolStateMachine : public tools::StateMachine
    {
    public:
        GraphViewToolStateMachine();
        void tick() override;
    };
}