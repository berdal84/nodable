#include "GraphView.h"

#include <algorithm>
#include "tools/core/types.h"
#include "tools/core/log.h"
#include "tools/core/System.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/core/math.h"
#include "tools/gui/Color.h"

#include "ndbl/core/Graph.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/NodeUtils.h"
#include "ndbl/core/Scope.h"
#include "ndbl/core/Slot.h"
#include "ndbl/core/Interpreter.h"

#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "SlotView.h"
#include "ndbl/core/ComponentFactory.h"
#include "tools/core/StateMachine.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<GraphView>("GraphView");
}

// Popup name
constexpr const char* CONTEXT_POPUP    = "GraphView.ContextMenuPopup";
// Tool names
constexpr const char* CURSOR_STATE     = "Cursor Tool";
constexpr const char* ROI_STATE        = "Selection Tool";
constexpr const char* DRAG_STATE       = "Drag Node Tool";
constexpr const char* VIEW_PAN_STATE   = "Grab View Tool";
constexpr const char* LINE_STATE       = "Line Tool";

GraphView::GraphView(Graph* graph)
: m_graph(graph)
, m_state_machine(this)
{
    ASSERT(graph != nullptr)

    m_state_machine.add_state(CURSOR_STATE);
    m_state_machine.bind_tick(CURSOR_STATE, &GraphView::cursor_state_tick);
    m_state_machine.set_default_state(CURSOR_STATE);

    m_state_machine.add_state(ROI_STATE);
    m_state_machine.bind_enter(ROI_STATE, &GraphView::roi_state_enter);
    m_state_machine.bind_tick(ROI_STATE, &GraphView::roi_state_tick);

    m_state_machine.add_state(DRAG_STATE);
    m_state_machine.bind_enter(DRAG_STATE, &GraphView::drag_state_enter);
    m_state_machine.bind_tick(DRAG_STATE, &GraphView::drag_state_tick);

    m_state_machine.add_state(VIEW_PAN_STATE);
    m_state_machine.bind_tick(VIEW_PAN_STATE, &GraphView::view_pan_state_tick);

    m_state_machine.add_state(LINE_STATE);
    m_state_machine.bind_enter(LINE_STATE, &GraphView::line_state_enter);
    m_state_machine.bind_tick(LINE_STATE, &GraphView::line_state_tick);
    m_state_machine.bind_leave(LINE_STATE, &GraphView::line_state_leave);


    m_state_machine.start();

    // When a new node is added
    graph->on_add.connect(
        [this](Node* node) -> void
        {
            // Add a NodeView and Physics component
            ComponentFactory* component_factory = get_component_factory();
            auto nodeview = component_factory->create<NodeView>();
            m_base_view.add_child(nodeview->base_view());
            auto physics  = component_factory->create<Physics>( nodeview );
            node->add_component( nodeview );
            node->add_component( physics );
        }
    );
}


ImGuiID make_wire_id(const Slot *ptr1, const Slot *ptr2)
{
    string128 id;
    id.append_fmt("wire %zu->%zu", ptr1, ptr2);
    return ImGui::GetID(id.c_str());
}

void GraphView::draw_wire_from_slot_to_pos(SlotView *from, const Vec2 &end_pos)
{
    VERIFY(from != nullptr, "from slot can't be nullptr")

    Config* cfg = get_config();

    // Style

    ImGuiEx::WireStyle style;
    style.shadow_color = cfg->ui_codeflow_shadowColor,
            style.roundness = 0.f;

    if (from->get_slot().type() == SlotFlag_TYPE_CODEFLOW) {
        style.color = cfg->ui_codeflow_color,
                style.thickness = cfg->ui_slot_rectangle_size.x * cfg->ui_codeflow_thickness_ratio;
    } else {
        style.color = cfg->ui_node_borderHighlightedColor;
        style.thickness = cfg->ui_wire_bezier_thickness;
    }

    // Draw

    ImGuiID id = make_wire_id(&from->get_slot(), nullptr);
    Vec2 start_pos = from->get_pos(SCREEN_SPACE);

    BezierCurveSegment segment{
            start_pos, start_pos,
            end_pos, end_pos
    }; // straight line

    ImGuiEx::DrawWire(id, ImGui::GetWindowDrawList(), segment, style);
}

bool GraphView::draw()
{
    m_base_view.begin_draw();
    m_hovered = {};
    Config*         cfg                    = get_config();
    Interpreter*    interpreter            = get_interpreter();
    bool            changed                = false;
    ImDrawList*     draw_list              = ImGui::GetWindowDrawList();
    const bool      enable_edition         = interpreter->is_program_stopped();
    std::vector<Node*> node_registry       = m_graph->get_node_registry();

    // Draw Grid
    ImGuiEx::Grid(
            cfg->ui_grid_size,
            cfg->ui_grid_subdiv_count,
            ImGui::GetColorU32(cfg->ui_graph_grid_color_major),
            ImGui::GetColorU32(cfg->ui_graph_grid_color_minor));

    // Draw Wires (code flow ONLY)
    const ImGuiEx::WireStyle code_flow_style{
            cfg->ui_codeflow_color,
            cfg->ui_codeflow_color, // hover
            cfg->ui_codeflow_shadowColor,
            cfg->ui_codeflow_thickness(),
            0.0f
    };
    for (Node *each_node: node_registry)
    {
        NodeView *each_view = NodeView::substitute_with_parent_if_not_visible(each_node->get_component<NodeView>());

        if (!each_view) {
            continue;
        }

        std::vector<Slot *> slots = each_node->filter_slots(SlotFlag_NEXT);
        for (size_t slot_index = 0; slot_index < slots.size(); ++slot_index)
        {
            Slot *slot = slots[slot_index];

            if (slot->empty())
            {
                continue;
            }

            for (const auto &adjacent_slot: slot->adjacent())
            {
                Node *each_successor_node = adjacent_slot->get_node();
                NodeView *each_successor_view = NodeView::substitute_with_parent_if_not_visible(
                        each_successor_node->get_component<NodeView>());

                if ( each_successor_view == nullptr )
                    continue;
                if ( each_view->visible() == false )
                    continue;
                if ( each_successor_view->visible() == false )
                    continue;

                SlotView *tail = slot->get_view();
                SlotView *head = adjacent_slot->get_view();

                ImGuiID id = make_wire_id(slot, adjacent_slot);
                Vec2 tail_pos = tail->get_pos(SCREEN_SPACE);
                Vec2 head_pos = head->get_pos(SCREEN_SPACE);
                BezierCurveSegment segment{
                        tail_pos,
                        tail_pos,
                        head_pos,
                        head_pos,
                };
                ImGuiEx::DrawWire(id, draw_list, segment, code_flow_style);
                if (ImGui::GetHoveredID() == id && m_hovered.empty() )
                    m_hovered = {tail, head};
            }
        }
    }

    // Draw Wires (regular)
    const ImGuiEx::WireStyle default_wire_style{
            cfg->ui_wire_color,
            cfg->ui_wire_color, // hover
            cfg->ui_wire_shadowColor,
            cfg->ui_wire_bezier_thickness,
            cfg->ui_wire_bezier_roundness.x // roundness min
    };
    for (auto each_node: node_registry)
    {
        for (const Slot *slot: each_node->filter_slots(SlotFlag_OUTPUT))
        {
            for(const Slot* adjacent_slot : slot->adjacent())
            {
                if (adjacent_slot == nullptr)
                    continue;

                auto *node_view         = slot->get_node()->get_component<NodeView>();
                auto *adjacent_nodeview = adjacent_slot->get_node()->get_component<NodeView>();

                if ( node_view->visible() == false )
                    continue;
                if ( adjacent_nodeview->visible() == false )
                    continue;

                ImGuiEx::WireStyle style    = default_wire_style;
                SlotView* slotview          = slot->get_view();
                SlotView* adjacent_slotview = adjacent_slot->get_view();

                const Vec2 start_pos = slotview->get_pos(SCREEN_SPACE);
                const Vec2 end_pos = adjacent_slotview->get_pos(SCREEN_SPACE);

                const Vec2 signed_dist = end_pos - start_pos;
                float lensqr_dist = signed_dist.lensqr();

                float roundness = 20.f;
                if ( signed_dist.y < 0.f )
                    roundness = 100.f;

                BezierCurveSegment segment{
                        start_pos,
                        start_pos + slotview->get_normal() * roundness,
                        end_pos + adjacent_slotview->get_normal() * roundness,
                        end_pos
                };

                // do not draw long lines between a variable value
                if (is_selected(node_view) ||
                    is_selected(adjacent_nodeview))
                {
                    style.color.w *= wave(0.5f, 1.f, (float) App::get_time(), 10.f);
                }
                else
                {
                    // transparent depending on wire length
                    if (lensqr_dist > cfg->ui_wire_bezier_fade_lensqr_range.x)
                    {
                        float factor = (lensqr_dist - cfg->ui_wire_bezier_fade_lensqr_range.x) /
                                       (cfg->ui_wire_bezier_fade_lensqr_range.y - cfg->ui_wire_bezier_fade_lensqr_range.x);
                        style.color = Vec4::lerp(style.color, Vec4(0, 0, 0, 0), factor);
                        style.shadow_color = Vec4::lerp(style.shadow_color, Vec4(0, 0, 0, 0), factor);
                    }
                }

                // draw the wire if necessary
                if (style.color.w != 0.f)
                {
                     if (slot->has_flags(SlotFlag_TYPE_CODEFLOW))
                     {
                        style.thickness *= 3.0f;
                        // style.roundness *= 0.25f;
                     }

                    // TODO: this block is repeated twice
                    ImGuiID id = make_wire_id(&slotview->get_slot(), adjacent_slot);
                    ImGuiEx::DrawWire(id, draw_list, segment, style);
                    if (ImGui::GetHoveredID() == id && m_hovered.empty())
                        m_hovered = {slotview, adjacent_slotview};
                }
            }
        }
    }

    // Draw NodeViews
    for (NodeView *nodeview: get_all_nodeviews())
    {
        if ( nodeview->visible() == false )
            continue;

        changed |= nodeview->draw();

        if ( nodeview->hovered() ) // no check if something else is hovered, last node always win against an edge
        {
            if ( nodeview->m_hovered_slotview != nullptr)
            {
                m_hovered = {nodeview->m_hovered_slotview};
            }
            else
                m_hovered = {nodeview};
        }

        // VM Cursor (scroll to the next node when VM is debugging)
        if (interpreter->is_debugging())
            if (interpreter->is_next_node(nodeview->get_owner()))
                ImGui::SetScrollHereY();
    }

    // Virtual Machine cursor
    if (interpreter->is_program_running())
    {
        const Node* node = interpreter->get_next_node();
        if (const NodeView* view = node->get_component<NodeView>())
        {
            Vec2 left = view->get_rect().left();
            Vec2 interpreter_cursor_pos = Vec2::round(left);
            draw_list->AddCircleFilled(interpreter_cursor_pos, 5.0f, ImColor(255, 0, 0));

            Vec2 linePos = interpreter_cursor_pos + Vec2(-10.0f, 0.5f);
            linePos += Vec2(sin(float(App::get_time()) * 12.0f) * 4.0f, 0.f); // wave
            float size = 20.0f;
            float width = 2.0f;
            ImColor color = ImColor(255, 255, 255);

            // arrow ->
            draw_list->AddLine(linePos - Vec2(1.f, 0.0f), linePos - Vec2(size, 0.0f), color, width);
            draw_list->AddLine(linePos, linePos - Vec2(size * 0.5f, -size * 0.5f), color, width);
            draw_list->AddLine(linePos, linePos - Vec2(size * 0.5f, size * 0.5f), color, width);
        }
    }

    m_state_machine.tick();

    // Debug Infos
    if (cfg->tools_cfg->runtime_debug)
    {
        if (ImGui::Begin("GraphViewToolStateMachine"))
        {
            ImGui::Text("current_tool:         %s", m_state_machine.get_current_state_name());
            ImGui::Text("m_focused.type:       %i", m_focused.type );
            ImGui::Text("m_hovered.type:       %i", m_hovered.type );
            Vec2 mouse_pos = ImGui::GetMousePos();
            ImGui::Text("m_mouse_pos:          (%f, %f)", mouse_pos.x, mouse_pos.y);
        }
        ImGui::End();
    }

    // add some empty space
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

	return changed;
}

bool GraphView::update(float delta_time, u16_t subsample_count)
{
    const float subsample_delta_time = delta_time / float(subsample_count);
    for(u16_t i = 0; i < subsample_count; i++)
        update( subsample_delta_time );
    return true;
}

bool GraphView::update(float delta_time)
{
    // 1. Update Physics Components
    std::vector<Physics*> physics_components = NodeUtils::get_components<Physics>( m_graph->get_node_registry() );
    // 1.1 Apply constraints (but apply no translation, we want to be sure order does no matter)
    for (auto physics_component : physics_components)
    {
        physics_component->apply_constraints(delta_time);
    }
    // 1.3 Apply forces (translate views)
    for(auto physics_component : physics_components)
    {
        physics_component->apply_forces(delta_time);
    }

    // 2. Update NodeViews
    std::vector<NodeView*> nodeview_components = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    for (auto eachView : nodeview_components)
    {
        eachView->update(delta_time);
    }

    return true;
}

bool GraphView::update()
{
    Config* cfg = get_config();
    return update( ImGui::GetIO().DeltaTime, cfg->ui_node_animation_subsample_count );
}

void GraphView::frame_views(const std::vector<NodeView*>& _views, bool _align_top_left_corner)
{
    if (_views.empty())
    {
        LOG_VERBOSE("GraphView", "Unable to frame views vector. Reason: is empty.\n")
        return;
    }

    Rect frame = m_base_view.get_content_region();

    // Get views' bbox
    Rect views_bbox = NodeView::get_rect(_views);

    // align
    Vec2 delta;
    if (_align_top_left_corner)
    {
        // Align with the top-left corner
        views_bbox.expand(Vec2(20.0f ) ); // add a padding to avoid alignment too close from the border
        delta = frame.top_left() - views_bbox.top_left();
    }
    else
    {
        // Align the center of the node rectangle with the frame center
        delta = frame.center() - views_bbox.center();
    }

    // apply the translation
    // TODO: Instead of applying a translation to all views, we could translate a Camera.
    auto node_views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    NodeView::translate(get_all_nodeviews(), delta);
}

void GraphView::unfold()
{
    Config* cfg = get_config();
    update( cfg->graph_unfold_dt, cfg->graph_unfold_iterations );
}

void GraphView::add_action_to_node_menu(Action_CreateNode* _action )
{
    m_create_node_menu.add_action(_action);
}

void GraphView::frame_nodes(FrameMode mode )
{
    switch (mode)
    {
        case FRAME_ALL:
        {
            if(m_graph->is_empty())
                return;

            // frame the root's view (top-left corner)
            auto& root_view = *m_graph->get_root()->get_component<NodeView>();
            frame_views( {&root_view}, true);
            break;
        }

        case FRAME_SELECTION_ONLY:
        {
            if ( !m_selected_nodeview.empty())
                frame_views(m_selected_nodeview, false);
            break;
        }
        default:
            VERIFY(false, "unhandled case!")
    }
}

void GraphView::set_selected(const NodeViewVec& views, SelectionMode mode )
{
    NodeViewVec curr_selection = m_selected_nodeview;
    if ( mode == SelectionMode_REPLACE )
    {
        m_selected_nodeview.clear();
        for(auto& each : curr_selection )
            each->set_selected(false);
    }

    for(auto& each : views)
    {
        m_selected_nodeview.emplace_back(each);
        each->set_selected();
    }

    EventPayload_NodeViewSelectionChange event{ m_selected_nodeview, curr_selection };
    get_event_manager()->dispatch<Event_SelectionChange>(event);
}

const GraphView::NodeViewVec& GraphView::get_selected() const
{
    return m_selected_nodeview;
}

bool GraphView::is_selected(NodeView* view) const
{
    return std::find( m_selected_nodeview.begin(), m_selected_nodeview.end(), view) != m_selected_nodeview.end();
}

bool GraphView::selection_empty() const
{
    return m_selected_nodeview.empty();
}

void GraphView::reset()
{
    if ( m_graph->is_empty() )
        return;

    // unfold the graph (does not work great when nodes are rendered for the first time)
    unfold();

    // make sure views are outside viewable rectangle (to avoid flickering)
    Vec2 far_outside = Vec2(-1000.f, -1000.0f);
    NodeView::translate(get_all_nodeviews(), far_outside);

    // frame all (33ms delayed to ensure layout is correct)
    get_event_manager()->dispatch_delayed<Event_FrameSelection>( 33, { FRAME_ALL } );
}

std::vector<NodeView*> GraphView::get_all_nodeviews() const
{
     return NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
}

bool GraphView::has_an_active_tool() const
{
    return !m_state_machine.has_default_state();
}

void GraphView::reset_all_properties()
{
    for( NodeView* each : get_all_nodeviews() )
        for( auto& [_, property_view] : each->m_property_views__all )
            property_view->reset();
}

Graph *GraphView::get_graph() const
{
    return m_graph;
}

//-----------------------------------------------------------------------------

void GraphView::draw_create_node_context_menu(CreateNodeCtxMenu& menu, SlotView* dragged_slotview)
{
    ImGuiEx::ColoredShadowedText( Vec2(1, 1), Color(0, 0, 0, 255), Color(255, 255, 255, 127), "Create new node :");
    ImGui::Separator();

    if (Action_CreateNode* triggered_action = menu.draw_search_input( dragged_slotview, 10))
    {
        // Generate an event from this action, add some info to the state and dispatch it.
        auto event                     = triggered_action->make_event();
        event->data.graph              = get_graph();
        event->data.active_slotview    = dragged_slotview;
        event->data.desired_screen_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
        get_event_manager()->dispatch(event);
        ImGui::CloseCurrentPopup();
    }
}

//-----------------------------------------------------------------------------

void GraphView::drag_state_enter()
{
    for(auto& each : m_selected_nodeview)
        each->set_pinned();
}

void GraphView::drag_state_tick()
{
    Vec2 delta = ImGui::GetMouseDragDelta();
    for (auto &node_view: get_selected() )
        node_view->translate(delta);

    ImGui::ResetMouseDragDelta();

    if ( ImGui::IsMouseReleased(0) )
        m_state_machine.exit_state();
}


//-----------------------------------------------------------------------------

void GraphView::view_pan_state_tick()
{
    // The code is very similar to drag_state_tick, however it should not be. Indeed, we hack a little here
    // by translating all the nodes instead of translating the graphview content...

    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

    Vec2 delta = ImGui::GetMouseDragDelta();
    for (auto &node_view: get_all_nodeviews() )
        node_view->translate(delta);

    ImGui::ResetMouseDragDelta();

    if ( ImGui::IsMouseReleased(0) )
        m_state_machine.exit_state();
}

//-----------------------------------------------------------------------------

void GraphView::cursor_state_tick()
{
    switch (m_hovered.type)
    {
        case ViewItemType_SLOT:
        {
            if (ImGui::IsMouseDragging(0, 0.f))
            {
                m_focused = m_hovered;
                m_state_machine.change_state(LINE_STATE);
            }
            break;
        }

        case ViewItemType_NODE:
        {
            if (ImGui::IsMouseReleased(0) )
            {
                // TODO: handle remove!
                // Add/Remove/Replace selection
                SelectionMode flags = SelectionMode_REPLACE;
                if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
                    flags = SelectionMode_ADD;
                set_selected({m_hovered.nodeview}, flags);
                m_focused = m_hovered;
            }
            else if (ImGui::IsMouseDoubleClicked(0))
            {
                m_hovered.nodeview->expand_toggle();
                m_focused = m_hovered;
            }
            else if (ImGui::IsMouseDragging(0))
            {
                if (!m_hovered.nodeview->selected())
                    set_selected({m_hovered.nodeview});
                m_state_machine.change_state(DRAG_STATE);
            }
            break;
        }

        case ViewItemType_EDGE:
        {
            if (ImGui::IsMouseDragging(0, 0.1f))
                m_focused = m_hovered;
            else if (ImGui::IsMouseClicked(1))
            {
                m_focused = m_hovered;
                ImGui::OpenPopup(CONTEXT_POPUP);
            }
            break;
        }

        case ViewItemType_NULL:
        {
            if ( ImGui::IsWindowHovered(ImGuiFocusedFlags_ChildWindows) )
            {
                if (ImGui::IsMouseClicked(0))
                    set_selected({}, SelectionMode_REPLACE); // Deselect All (Click on the background)
                else if (ImGui::IsMouseReleased(0))
                    m_focused = {};
                else if (ImGui::IsMouseClicked(1))
                    ImGui::OpenPopup(CONTEXT_POPUP);
                else if (ImGui::IsMouseDragging(0))
                    m_state_machine.change_state(ImGui::IsKeyDown(ImGuiKey_Space) ? VIEW_PAN_STATE : ROI_STATE);
            }

            break;
        }

        default:
            VERIFY(false, "Unhandled case, must be implemented!")
    }

    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        if ( ImGui::IsWindowAppearing())
            m_create_node_menu.flag_to_be_reset();

        if ( m_hovered.empty() )
            draw_create_node_context_menu(m_create_node_menu);
        ImGui::EndPopup();
    }
}

//-----------------------------------------------------------------------------

void GraphView::line_state_enter()
{
    ASSERT(m_focused.type == ViewItemType_SLOT)
    ASSERT(m_focused.slotview != nullptr)
}

void GraphView::line_state_tick()
{
    Vec2 mouse_pos_snapped = m_hovered.type == ViewItemType_SLOT ? m_hovered.slotview->get_pos(SCREEN_SPACE)
                                                                 : Vec2{ImGui::GetMousePos()};

    // Contextual menu
    if ( ImGui::BeginPopup(CONTEXT_POPUP) )
    {
        mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        if ( ImGui::IsWindowAppearing() )
            m_create_node_menu.flag_to_be_reset();

        if ( m_hovered.empty() )
            draw_create_node_context_menu(m_create_node_menu, m_focused.slotview);

        if ( ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1) )
            m_state_machine.exit_state();

        ImGui::EndPopup();
    }
    else if ( ImGui::IsMouseReleased(0) )
    {
        switch (m_hovered.type)
        {
            case ViewItemType_SLOT:
            {
                auto event = new Event_SlotDropped();
                event->data.first  = &m_focused.slotview->get_slot();
                event->data.second = &m_hovered.slotview->get_slot();
                get_event_manager()->dispatch(event);
                m_state_machine.exit_state();
                break;
            }

            case ViewItemType_EDGE:
            case ViewItemType_NODE:
            case ViewItemType_NULL: // ...on background
            {
                ImGui::OpenPopup(CONTEXT_POPUP);
                break;
            }
        }
    }

    // Draw a temporary wire from focused/dragged slotview to the mouse cursor
    draw_wire_from_slot_to_pos(m_focused.slotview, mouse_pos_snapped );
}

void GraphView::line_state_leave()
{
    m_focused = {};
}

//-----------------------------------------------------------------------------

void GraphView::roi_state_enter()
{
    m_roi_state_start_pos = ImGui::GetMousePos();
    m_roi_state_end_pos   = ImGui::GetMousePos();;
}

void GraphView::roi_state_tick()
{
    m_roi_state_end_pos = ImGui::GetMousePos();

    // Get normalized ROI rectangle
    Rect roi = Rect::normalize({m_roi_state_start_pos, m_roi_state_end_pos});

    // Expand to avoid null area
    const int roi_border_width = 2;
    roi.expand(Vec2{roi_border_width*0.5f});

    // Draw the ROI rectangle
    float alpha = wave(0.5f, 0.75f, (float) App::get_time(), 10.f);
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRect(roi.min, roi.max, ImColor(1.f, 1.f, 1.f, alpha), roi_border_width, ImDrawFlags_RoundCornersAll , roi_border_width );

    if (ImGui::IsMouseReleased(0))
    {
        // Select the views included in the ROI
        std::vector<NodeView*> nodeview_in_roi;
        for (NodeView* nodeview: get_all_nodeviews())
        {
            if (Rect::contains(roi, nodeview->get_rect()))
                nodeview_in_roi.emplace_back(nodeview);
        }
        bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                               ImGui::IsKeyDown(ImGuiKey_RightCtrl);
        SelectionMode flags = control_pressed ? SelectionMode_ADD : SelectionMode_REPLACE;
        set_selected(nodeview_in_roi, flags);

        m_state_machine.exit_state();
    }
}
