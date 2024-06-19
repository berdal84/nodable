#include "GraphView.h"

#include <algorithm>
#include "tools/core/types.h"
#include "tools/core/log.h"
#include "tools/core/system.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/core/math.h"
#include "tools/core/Color.h"

#include "ndbl/core/Graph.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/NodeUtils.h"
#include "ndbl/core/Scope.h"
#include "ndbl/core/Slot.h"
#include "ndbl/core/VirtualMachine.h"

#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"
#include "SlotView.h"
#include "ndbl/core/ComponentFactory.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<GraphView>("GraphView").extends<View>();
}

GraphView::GraphView(Graph* graph)
    : View()
    , m_graph(graph)
{
    ASSERT(graph)

    // When a new node is added
    graph->on_add.connect(
        [this](Node* node) -> void
        {
            // Add a NodeView and Physics component
            ComponentFactory* component_factory = get_component_factory();
            auto nodeview = component_factory->create<NodeView>();
            this->add_child(nodeview);
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
    EXPECT(from != nullptr, "from slot can't be nullptr")

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
    View::draw();

    Config*         cfg                    = get_config();
    VirtualMachine* virtual_machine        = get_virtual_machine();
    bool            changed                = false;
    ImDrawList*     draw_list              = ImGui::GetWindowDrawList();
    const bool      enable_edition         = virtual_machine->is_program_stopped();
    std::vector<Node*> node_registry       = m_graph->get_node_registry();
    const ImVec2    mouse_pos              = ImGui::GetMousePos();
    Item            hovered                = Item{};

    auto draw_grid = [](ImDrawList *draw_list) -> void {
        Config *cfg = get_config();
        Rect area = ImGuiEx::GetContentRegion(SCREEN_SPACE);
        int grid_subdiv_size = cfg->ui_grid_subdiv_size();
        int vertical_line_count = int(area.size().x) / grid_subdiv_size;
        int horizontal_line_count = int(area.size().y) / grid_subdiv_size;
        Vec4 grid_color = cfg->ui_graph_grid_color_major;
        Vec4 grid_color_light = cfg->ui_graph_grid_color_minor;

        for (int coord = 0; coord <= vertical_line_count; ++coord) {
            float pos = area.tl().x + float(coord) * float(grid_subdiv_size);
            Vec2 line_start{pos, area.tl().y};
            Vec2 line_end{pos, area.bl().y};
            bool is_major = coord % cfg->ui_grid_subdiv_count == 0;
            ImColor color{is_major ? grid_color : grid_color_light};
            draw_list->AddLine(line_start, line_end, color);
        }

        for (int coord = 0; coord <= horizontal_line_count; ++coord) {
            float pos = area.tl().y + float(coord) * float(grid_subdiv_size);
            Vec2 line_start{area.tl().x, pos};
            Vec2 line_end{area.br().x, pos};
            bool is_major = coord % cfg->ui_grid_subdiv_count == 0;
            ImColor color{is_major ? grid_color : grid_color_light};
            draw_list->AddLine(line_start, line_end, color);
        }
    };

    // Background Grid
    draw_grid(draw_list);

    // Draw Wires (code flow ONLY)
    const ImGuiEx::WireStyle code_flow_style{
            cfg->ui_codeflow_color,
            cfg->ui_codeflow_color, // hover
            cfg->ui_codeflow_shadowColor,
            cfg->ui_codeflow_thickness(),
            0.0f
    };
    for (Node *each_node: node_registry) // TODO: Consider iterating over all the DirectedEdges instead
    {
        NodeView *each_view = NodeView::substitute_with_parent_if_not_visible(each_node->get_component<NodeView>());

        if (!each_view) {
            continue;
        }

        std::vector<Slot *> slots = each_node->filter_slots(SlotFlag_NEXT);
        for (size_t slot_index = 0; slot_index < slots.size(); ++slot_index) {
            Slot *slot = slots[slot_index];

            if (slot->empty()) {
                continue;
            }

            for (const auto &adjacent_slot: slot->adjacent()) {
                Node *each_successor_node = adjacent_slot->get_node();
                NodeView *each_successor_view = NodeView::substitute_with_parent_if_not_visible(
                        each_successor_node->get_component<NodeView>());

                if (!each_successor_view)
                    continue;
                if (!each_view->visible)
                    continue;
                if (!each_successor_view->visible)
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
                if (ImGui::GetHoveredID() == id)
                    hovered = EdgeItem{tail, head};
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
    for (auto each_node: node_registry) {
        for (const Slot *slot: each_node->filter_slots(SlotFlag_OUTPUT)) {
            ImGuiEx::WireStyle style = default_wire_style;
            Slot *adjacent_slot = slot->first_adjacent();

            if (adjacent_slot == nullptr)
                continue;

            NodeView *node_view = slot->get_node()->get_component<NodeView>();
            NodeView *adjacent_nodeview = adjacent_slot->get_node()->get_component<NodeView>();

            if (!node_view->visible)
                continue;
            if (!adjacent_nodeview->visible)
                continue;

            SlotView *slotview = slot->get_view();
            SlotView *adjacent_slotview = adjacent_slot->get_view();

            const Vec2 &start_pos = slotview->get_pos(SCREEN_SPACE);
            const Vec2 &end_pos = adjacent_slotview->get_pos(SCREEN_SPACE);

            const Vec2 signed_dist = end_pos - start_pos;
            float lensqr_dist = signed_dist.lensqr();
            const Vec2 half_signed_dist = signed_dist * 0.5f;

            BezierCurveSegment segment{
                    start_pos,
                    start_pos + half_signed_dist * slotview->get_normal(),
                    end_pos + half_signed_dist * adjacent_slotview->get_normal(),
                    end_pos
            };

            // do not draw long lines between a variable value
            if (is_selected(node_view) ||
                is_selected(adjacent_nodeview)) {
                style.color.w *= wave(0.5f, 1.f, (float) BaseApp::elapsed_time(), 10.f);
            } else {
                // transparent depending on wire length
                if (lensqr_dist > cfg->ui_wire_bezier_fade_lensqr_range.x) {
                    float factor = (lensqr_dist - cfg->ui_wire_bezier_fade_lensqr_range.x) /
                                   (cfg->ui_wire_bezier_fade_lensqr_range.y - cfg->ui_wire_bezier_fade_lensqr_range.x);
                    style.color = Vec4::lerp(style.color, Vec4(0, 0, 0, 0), factor);
                    style.shadow_color = Vec4::lerp(style.shadow_color, Vec4(0, 0, 0, 0), factor);
                }
            }

            // draw the wire if necessary
            if (style.color.w != 0.f) {
                style.roundness = lerp(
                        cfg->ui_wire_bezier_roundness.x, // min
                        cfg->ui_wire_bezier_roundness.y, // max
                        1.0f - normalize(lensqr_dist, 100.0f, 10000.0f)
                        + 1.0f - normalize(lensqr_dist, 0.0f, 200.0f)
                );

                if (slot->has_flags(SlotFlag_TYPE_CODEFLOW)) {
                    style.thickness *= 3.0f;
                    // style.roundness *= 0.25f;
                }

                // TODO: this block is repeated twice
                ImGuiID id = make_wire_id(&slotview->get_slot(), adjacent_slot);
                ImGuiEx::DrawWire(id, draw_list, segment, style);
                if (ImGui::GetHoveredID() == id)
                    hovered = EdgeItem{slotview, adjacent_slotview};
            }
        }
    }

    // Draw NodeViews
    for (NodeView *nodeview: get_all_nodeviews())
    {
        if (!nodeview->visible)
            continue;

        changed |= nodeview->draw();

        if (nodeview->hovered)
            hovered = NodeViewItem{nodeview};

        // VM Cursor (scroll to the next node when VM is debugging)
        if (virtual_machine->is_debugging())
            if (virtual_machine->is_next_node(nodeview->get_owner()))
                ImGui::SetScrollHereY();
    }

    // Hovering a SlotView is always the priority
    if ( hovered.type == ItemType_NODEVIEW && hovered.node.view->m_hovered_slotview != nullptr )
        hovered = SlotViewItem{hovered.node.view->m_hovered_slotview};

    // Virtual Machine cursor
    if (virtual_machine->is_program_running()) {
        Node *node = virtual_machine->get_next_node();
        if (auto *view = node->get_component<NodeView>()) {
            Vec2 left = view->get_rect(SCREEN_SPACE).left();
            Vec2 vm_cursor_pos = Vec2::round(left);
            draw_list->AddCircleFilled(vm_cursor_pos, 5.0f, ImColor(255, 0, 0));

            Vec2 linePos = vm_cursor_pos + Vec2(-10.0f, 0.5f);
            linePos += Vec2(sin(float(BaseApp::elapsed_time()) * 12.0f) * 4.0f, 0.f); // wave
            float size = 20.0f;
            float width = 2.0f;
            ImColor color = ImColor(255, 255, 255);

            // arrow ->
            draw_list->AddLine(linePos - Vec2(1.f, 0.0f), linePos - Vec2(size, 0.0f), color, width);
            draw_list->AddLine(linePos, linePos - Vec2(size * 0.5f, -size * 0.5f), color, width);
            draw_list->AddLine(linePos, linePos - Vec2(size * 0.5f, size * 0.5f), color, width);
        }
    }

    // Update tool context
    m_context.mouse_pos  = mouse_pos;
    m_context.graph_view = this;
    m_context.hovered    = hovered;
    m_context.draw_list  = draw_list;
    bool snap_focused_slotview = m_context.focused.type == ItemType_SLOTVIEW
                              && m_context.focused.slot.view != nullptr;
    if ( snap_focused_slotview )
        m_context.mouse_pos_snapped = m_context.focused.slot.view->get_pos(SCREEN_SPACE);
    else
        m_context.mouse_pos_snapped = mouse_pos;

    // Update tool state
    m_tool.tick();
    m_tool.draw();

    // add some empty space
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

    // Debug Infos
    if ( cfg->tools_cfg->runtime_debug )
    {
        if ( ImGui::Begin("GraphView debug info" ) )
        {
            ImGui::Text("m_context.focused.type:       %i", m_context.focused.type);
            ImGui::Text("m_tool.type:                  %i", m_tool.tool_type());
            ImGui::Text("m_context.hovered.type:       %i", m_context.hovered.type);
            ImGui::Text("m_context.mouse_pos:          (%f, %f)", m_context.mouse_pos.x, m_context.mouse_pos.y);
            ImGui::Text("m_context.mouse_pos (snapped):(%f, %f)", m_context.mouse_pos_snapped.x, m_context.mouse_pos_snapped.y);
        }
        ImGui::End();
    }

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
        physics_component->apply_forces(delta_time, false);
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

    Rect frame = get_content_region(SCREEN_SPACE);

    // Get views' bbox
    Rect views_bbox = NodeView::get_rect(_views, SCREEN_SPACE);

    // debug
    ImGuiEx::DebugRect(views_bbox.min, views_bbox.max, IM_COL32(0, 255, 0, 127 ), 5.0f );
    ImGuiEx::DebugRect( frame.min, frame.max, IM_COL32( 255, 255, 0, 127 ), 5.0f );

    // align
    Vec2 move;
    if (_align_top_left_corner)
    {
        // Align with the top-left corner
        views_bbox.expand(Vec2(20.0f ) ); // add a padding to avoid alignment too close from the border
        move = frame.tl() - views_bbox.tl();
    }
    else
    {
        // Align the center of the node rectangle with the frame center
        move = frame.center() - views_bbox.center();
    }

    // apply the translation
    // TODO: Instead of applying a translation to all views, we could translate a Camera.
    auto node_views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all(node_views, move, NodeViewFlag_NONE);

    // debug
    ImGuiEx::DebugLine(views_bbox.center(), views_bbox.center() + move, IM_COL32(255, 0, 0, 255 ), 20.0f);
}

void GraphView::translate_all(const std::vector<NodeView*>& _views, const Vec2& delta, NodeViewFlags flags )
{
    for (auto node_view : _views )
    {
        node_view->translate(delta, flags);
    }
}

void GraphView::unfold()
{
    Config* cfg = get_config();
    update( cfg->graph_unfold_dt, cfg->graph_unfold_iterations );
}

void GraphView::translate_all(const Vec2& delta)
{
    translate_all(get_all_nodeviews(), delta, NodeViewFlag_NONE);
}

void GraphView::add_action_to_context_menu( Action_CreateNode* _action )
{
    m_context.create_node_ctx_menu.add_action(_action);
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
            if ( !m_context.selected_nodeview.empty())
                frame_views(m_context.selected_nodeview, false);
            break;
        }
        default:
            EXPECT(false, "unhandled case!")
    }
}

void GraphView::set_selected(const NodeViewVec& views, SelectionMode mode )
{
    NodeViewVec curr_selection = m_context.selected_nodeview;
    if ( mode == SelectionMode_REPLACE )
    {
        m_context.selected_nodeview.clear();
        for(auto& each : curr_selection )
            each->selected = false;
    }

    for(auto& each : views)
    {
        m_context.selected_nodeview.emplace_back(each);
        each->selected = true;
    }

    EventPayload_NodeViewSelectionChange event{ m_context.selected_nodeview, curr_selection };
    EventManager& event_manager = EventManager::get_instance();
    event_manager.dispatch<Event_SelectionChange>(event);
}

const GraphView::NodeViewVec& GraphView::get_selected() const
{
    return m_context.selected_nodeview;
}

bool GraphView::is_selected(NodeView* view) const
{
    return std::find( m_context.selected_nodeview.begin(), m_context.selected_nodeview.end(), view) != m_context.selected_nodeview.end();
}

bool GraphView::selection_empty() const
{
    return m_context.selected_nodeview.empty();
}

void GraphView::reset()
{
    if ( m_graph->is_empty() )
        return;

    // unfold the graph (does not work great when nodes are rendered for the first time)
    unfold();

    // make sure views are outside viewable rectangle (to avoid flickering)
    auto views = NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
    translate_all(views, Vec2(-1000.f, -1000.0f), NodeViewFlag_NONE);

    // frame all (33ms delayed to ensure layout is correct)
    auto& event_manager = EventManager::get_instance();
    event_manager.dispatch_delayed<Event_FrameSelection>( 33, { FRAME_ALL } );
}

std::vector<NodeView*> GraphView::get_all_nodeviews() const
{
     return NodeUtils::get_components<NodeView>( m_graph->get_node_registry() );
}

bool GraphView::has_an_active_tool() const
{
    return m_tool.tool_type() != ToolType_CURSOR;
}

void GraphView::reset_all_properties()
{
    for( NodeView* each : get_all_nodeviews() )
        for( PropertyView* property_view : each->m_property_views )
            property_view->reset();
}

Graph *GraphView::get_graph() const
{
    return m_graph;
}
