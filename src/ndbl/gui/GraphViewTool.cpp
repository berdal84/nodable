#include "GraphViewTool.h"
#include "tools/gui/BaseApp.h"
#include "tools/core/math.h"
#include "imgui.h"
#include "NodeView.h"
#include "GraphView.h"
#include "tools/core/Color.h"
#include "Config.h"

using namespace ndbl;
using namespace tools;

Transition CursorToolToLineTool_Transition
{
    [](const State* curr_state)
    {
        if (curr_state->id != ToolType_CURSOR)
            return false;

        auto *current_tool = (GraphViewTool *) curr_state;
        if (current_tool->m_context.hovered.is<SlotViewItem>())
            if (ImGui::IsMouseDragging(0, 0.1f))
                return true;

        return false;
    },

    [](State* curr_state)
    {
        auto *current_tool = (GraphViewTool *) curr_state;
        return new LineTool(current_tool->m_context);
    }
};

Transition CursorToDragToolAll_Transition
{
    [](const State* curr_state)
    {
        if (curr_state->id != ToolType_CURSOR )
            return false;

        auto* current_tool = (GraphViewTool*)curr_state;

        if ( current_tool->m_context.hovered.empty() )
            if ( ImGui::IsMouseDragging(0, 0.1f) && current_tool->m_context.focused.is<SlotViewItem>() )
                if (ImGui::IsKeyDown(ImGuiKey_Space))
                    return true;
        return false;
    },

    [](State* curr_state)
    {
        auto *current_tool = (GraphViewTool *) curr_state;
        return new DragTool(current_tool->m_context, ndbl::DragTool::Mode::ALL);
    }
};

Transition CursorToolToDragToolSelection_Transition =
{
    [](const State* curr_state)
    {
        if (curr_state->id != ToolType_CURSOR )
            return false;

        auto* current_tool = (GraphViewTool*)curr_state;

        if ( current_tool->m_context.hovered.is<NodeViewItem>() )
        {
            if (ImGui::IsMouseClicked(0) && !current_tool->m_context.hovered.get<NodeViewItem>()->selected || ImGui::IsMouseDoubleClicked(0))
                return false;
            else if (ImGui::IsMouseDragging(0))
                return true;
        }
        return false;
    },

    [](State* curr_state)
    {
        auto* current_tool = (GraphViewTool*)curr_state;
        return new DragTool(current_tool->m_context, ndbl::DragTool::Mode::SELECTION);
    }
};

Transition CursorToolToROITool_Transition
{
    [](const State* curr_state)
    {
        auto* current_tool = (GraphViewTool*)curr_state;
        if (current_tool->id == ToolType_CURSOR )
            if ( current_tool->m_context.hovered.empty() )
                if (ImGui::IsMouseDragging(0, 0.1f) && !current_tool->m_context.focused.is<EdgeViewItem>() )
                    return true;
        return false;
    },
    [](State* curr_state)
    {
        auto *current_tool = (GraphViewTool *) curr_state;
        return new ROITool(current_tool->m_context);
    }
};


Transition DragToolToCursorTool_Transition
{
    [](const State* curr_tool)
    {
        if (curr_tool->id == ToolType_DRAG )
            return !ImGui::IsMouseDragging(0);
        return false;
    },
    [](State* curr_state)
    {
        auto *current_tool = (GraphViewTool *) curr_state;
        return new CursorTool(current_tool->m_context);
    }
};

Transition ROIToolToCursorTool_Transition
{
    [](const State* curr_tool)
    {
        if (curr_tool->id == ToolType_ROI )
            if (ImGui::IsMouseReleased(0))
                return true;
        return false;
    },

    [](State* curr_state)
    {
        auto *current_tool = (GraphViewTool *) curr_state;
        return new CursorTool(current_tool->m_context);
    }
};


//-----------------------------------------------------------------------------

void DragTool::on_enter()
{
    for(auto& each : m_context.selected_nodeview)
        each->set_pinned(true);
}

void DragTool::on_tick()
{
    Vec2 delta = ImGui::GetMouseDragDelta();
    if (delta.lensqr() < 1) // avoid updating when mouse is static
        return;

    NodeViewFlags flags = NodeViewFlag_IGNORE_SELECTED | NodeViewFlag_IGNORE_PINNED;
    if ( m_mode == DragTool::Mode::SELECTION )
    {
        for (auto &node_view: m_context.graph_view->get_selected() )
            node_view->translate(delta, flags);
    }
    else
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        for (auto &node_view: m_context.graph_view->get_all_nodeviews()) // TODO: if get_all_nodeviews() was returning a reference, we could merge the if/else branches
            node_view->translate(delta, flags);
    }

    ImGui::ResetMouseDragDelta();
}

DragTool::DragTool(GraphViewToolContext &ctx, DragTool::Mode mode)
        : GraphViewTool(ToolType_DRAG, ctx)
        , m_mode(mode)
{}

//-----------------------------------------------------------------------------

void CursorTool::on_tick()
{
    if ( !ImGui::IsWindowHovered(ImGuiFocusedFlags_ChildWindows) )
        return;

    if ( m_context.hovered.is<SlotViewItem>() )
    {
        if (ImGui::IsMouseReleased(2))
        {
            m_context.focused = m_context.hovered;
        }
    }
    else if ( m_context.hovered.is<NodeViewItem>() )
    {
        if (ImGui::IsMouseClicked(0) )
        {
            // TODO: handle remove!
            // Add/Remove/Replace selection
            SelectionMode flags = SelectionMode_REPLACE;
            if ( ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl) )
                flags = SelectionMode_ADD;

            std::vector<NodeView*> new_selection{m_context.hovered.get<NodeViewItem>()};
            m_context.graph_view->set_selected(new_selection, flags);
            m_context.focused = m_context.hovered;
        }
        if ( ImGui::IsMouseDoubleClicked(0) )
        {
            m_context.hovered.get<NodeViewItem>()->expand_toggle();
            m_context.focused = m_context.hovered;
        }
    }
    else if ( m_context.hovered.is<EdgeViewItem>() )
    {
        if (ImGui::IsMouseDragging(0, 0.1f) )
            m_context.focused = m_context.hovered;
        else if ( ImGui::IsMouseClicked(1) )
        {
            m_context.focused = m_context.hovered;
            ImGui::OpenPopup(GraphViewToolContext::POPUP_NAME);
        }
    }
    else if ( m_context.hovered.empty() )
    {
        if (ImGui::IsMouseClicked(0))
        {
            // Deselect All (Click on the background)
            m_context.graph_view->set_selected({}, SelectionMode_REPLACE);
        }
        else if (ImGui::IsMouseReleased(1))
        {
            // Open Context Menu
            m_context.focused = {};
            ImGui::OpenPopup(GraphViewToolContext::POPUP_NAME);
        }
    }
}

void CursorTool::draw()
{
    if ( m_context.begin_context_menu() )
    {
        bool show_search = m_context.hovered.empty();
        m_context.end_context_menu(show_search);
    }
}

CursorTool::CursorTool(GraphViewToolContext &ctx)
        : GraphViewTool(ToolType_CURSOR, ctx)
{}

//-----------------------------------------------------------------------------

void LineTool::on_tick()
{
    if ( !ImGui::IsWindowHovered(ImGuiFocusedFlags_ChildWindows) )
        return;
    ASSERT(m_dragged_slotview != nullptr)

    // Open popup to create a new node when dropped over background
    if (ImGui::IsMouseReleased(0))
    {
        if ( m_context.hovered.is<SlotViewItem>() )
        {
            auto &event_manager = EventManager::get_instance();
            auto event = new Event_SlotDropped();
            event->data.first = &m_dragged_slotview->get_slot();
            event->data.second = &m_context.hovered.get<SlotViewItem>()->get_slot();
            event_manager.dispatch(event);
        }
        else
        {
            m_context.open_popup();
        }
    }
}

void LineTool::draw()
{
    // Draw temp wire
    m_context.graph_view->draw_wire_from_slot_to_pos(m_dragged_slotview , m_context.mouse_pos_snapped() );
}

LineTool::LineTool(GraphViewToolContext &context)
        : GraphViewTool(ToolType_LINE, context)
        , m_dragged_slotview(context.hovered.get<SlotViewItem>())
{}

//-----------------------------------------------------------------------------

ROITool::ROITool(GraphViewToolContext &ctx)
        : GraphViewTool(ToolType_ROI, ctx)
        , m_start_pos(ctx.mouse_pos)
        , m_end_pos(ctx.mouse_pos)
{}

void ROITool::on_enter()
{

}

void ROITool::on_tick()
{
    // Update ROI second point
    m_end_pos = m_context.mouse_pos;
}

void ROITool::draw()
{
    // Get normalized ROI rectangle
    Rect roi = get_rect();

    // Expand to avoid null area
    const int roi_border_width = 2;
    roi.expand(Vec2{roi_border_width*0.5f});

    // Draw the ROI rectangle
    float alpha = wave(0.5f, 0.75f, (float) BaseApp::elapsed_time(), 10.f);
    m_context.draw_list->AddRect(roi.min, roi.max, ImColor(1.f, 1.f, 1.f, alpha), roi_border_width, ImDrawFlags_RoundCornersAll , roi_border_width );
}

void ROITool::on_leave()
{
    // Select the views included in the ROI
    std::vector<NodeView*> nodeview_in_roi;
    for (NodeView* nodeview: m_context.graph_view->get_all_nodeviews())
        if (Rect::contains( get_rect(), nodeview->get_rect(SCREEN_SPACE)))
            nodeview_in_roi.emplace_back(nodeview);

    bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                           ImGui::IsKeyDown(ImGuiKey_RightCtrl);
    SelectionMode flags = control_pressed ? SelectionMode_ADD
                                          : SelectionMode_REPLACE;
    m_context.graph_view->set_selected(nodeview_in_roi, flags);
}

tools::Rect ROITool::get_rect() const
{
    return tools::Rect::normalize({this->m_start_pos, this->m_end_pos });
}

//-----------------------------------------------------------------------------

tools::Vec2 GraphViewToolContext::mouse_pos_snapped() const
{
    if ( context_menu.open_this_frame )
        return context_menu.mouse_pos;
    if ( hovered.is<SlotViewItem>() )
        return hovered.get<SlotViewItem>()->get_pos(tools::SCREEN_SPACE);
    return mouse_pos;
}

bool GraphViewToolContext::begin_context_menu()
{
    // Context menu (draw)
    context_menu.open_last_frame = context_menu.open_this_frame;
    context_menu.open_this_frame = false;
    bool open = ImGui::BeginPopup(GraphViewToolContext::POPUP_NAME);
    if (open)
    {
        context_menu.mouse_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
    }
    context_menu.open_this_frame = open;
    return open;
}

void GraphViewToolContext::end_context_menu(bool show_search)
{
    if ( show_search )
    {
        if( !context_menu.open_last_frame )
            context_menu.node_menu.reset_state();

        ImGuiEx::ColoredShadowedText( Vec2(1, 1), Color(0, 0, 0, 255), Color(255, 255, 255, 127), "Create new node :");
        ImGui::Separator();

        SlotView* focused_slotview = get_focused_slotview();
        if (Action_CreateNode *triggered_action = context_menu.node_menu.draw_search_input( focused_slotview, 10))
        {
            // Generate an event from this action, add some info to the state and dispatch it.
            auto& event_manager            = EventManager::get_instance();
            auto event                     = triggered_action->make_event();
            event->data.graph              = graph_view->get_graph();
            event->data.active_slotview    = focused_slotview;
            event->data.desired_screen_pos = context_menu.mouse_pos;
            event_manager.dispatch(event);
            ImGui::CloseCurrentPopup();
        }
    }
    ImGui::EndPopup();
}

SlotView *GraphViewToolContext::get_focused_slotview() const
{
    if( focused.is<SlotViewItem>() )
        return focused.get<SlotViewItem>();
    return nullptr;
}

void GraphViewToolContext::open_popup() const
{
    ImGui::OpenPopup(POPUP_NAME);
}

//-----------------------------------------------------------------------------

GraphViewToolStateMachine::GraphViewToolStateMachine()
: tools::StateMachine()
{
    add_transition(&CursorToolToLineTool_Transition);
    add_transition(&CursorToDragToolAll_Transition);
    add_transition(&CursorToolToDragToolSelection_Transition);
    add_transition(&ROIToolToCursorTool_Transition);
    add_transition(&DragToolToCursorTool_Transition);
    add_transition(&CursorToolToROITool_Transition);
}

void GraphViewToolStateMachine::tick()
{
    StateMachine::tick();

    auto* current_tool = reinterpret_cast<GraphViewTool*>( m_current_state );
    current_tool->draw();

    // Debug Infos
    Config* cfg = get_config();
    if ( cfg->tools_cfg->runtime_debug )
    {
        if ( ImGui::Begin("GraphViewToolStateMachine" ) )
        {
            auto& context = current_tool->m_context;
            ImGui::Text("current_tool.id:           %i"      , (int) current_tool->id );
            ImGui::Text("context.focused.type:       %i"      , (int)context.focused.index() );
            ImGui::Text("context.hovered.type:       %i"      , (int)context.hovered.index() );
            ImGui::Text("context.mouse_pos:          (%f, %f)", context.mouse_pos.x, context.mouse_pos.y);
            ImGui::Text("context.mouse_pos (snapped):(%f, %f)", context.mouse_pos_snapped().x, context.mouse_pos_snapped().y);
        }
        ImGui::End();
    }
}

GraphViewTool::GraphViewTool(ToolType type, GraphViewToolContext &ctx)
        : tools::State(type)
        , m_context(ctx)
{}
