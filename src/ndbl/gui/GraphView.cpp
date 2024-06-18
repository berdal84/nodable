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

const char* k_CONTEXT_MENU_POPUP = "GraphView.ContextMenuPopup";

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

bool GraphView::draw() {
    View::draw();

    Config*         cfg                    = get_config();
    VirtualMachine* virtual_machine        = get_virtual_machine();
    bool            changed                = false;
    ImDrawList*     draw_list              = ImGui::GetWindowDrawList();
    const bool      enable_edition         = virtual_machine->is_program_stopped();
    std::vector<Node*> node_registry       = m_graph->get_node_registry();
    const ImVec2    mouse_pos              = ImGui::GetMousePos();
    ImVec2          mouse_pos_snapped      = ImGui::GetMousePos(); // snapped to any hovered slot
    Item            hovered                = Item{};

    auto make_wire_id = [](const Slot *ptr1, const Slot *ptr2) -> ImGuiID {
        string128 id;
        id.append_fmt("wire %zu->%zu", ptr1, ptr2);
        return ImGui::GetID(id.c_str());
    };

    auto draw_wire_from_slot_to_pos = [&](SlotView *from, const Vec2 &end_pos) -> void {
        EXPECT(from != nullptr, "from slot can't be nullptr")

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

        ImGuiEx::DrawWire(id, draw_list, segment, style);
    };

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
                    hovered = Edge_Item{tail, head};
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
                    hovered = Edge_Item{slotview, adjacent_slotview};
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
            hovered = NodeView_Item{nodeview};

        // VM Cursor (scroll to the next node when VM is debugging)
        if (virtual_machine->is_debugging())
            if (virtual_machine->is_next_node(nodeview->get_owner()))
                ImGui::SetScrollHereY();
    }

    // Hovering a SlotView is always the priority
    if ( hovered.type == ItemType_NODEVIEW && hovered.node.view->m_hovered_slotview != nullptr )
        hovered = SlotView_Item{hovered.node.view->m_hovered_slotview};

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

    // Update snapped_mouse_pos
    if (m_focused.type == ItemType_SLOTVIEW && m_focused.slot.view != nullptr)
        mouse_pos_snapped = m_focused.slot.view->get_pos(SCREEN_SPACE);

    // Tool Update
    // Here we must not change tool, use Post-Update instead.
    switch ( m_tool.type )
    {
        case ToolType_DRAG:
        {
            Vec2 delta = ImGui::GetMouseDragDelta();
            if (delta.lensqr() < 1) // avoid updating when mouse is static
                break;

            NodeViewFlags flags = NodeViewFlag_IGNORE_SELECTED | NodeViewFlag_IGNORE_PINNED;
            if (m_tool.drag.mode == DragNodeViews_Tool::Mode::SELECTION)
            {
                for (auto &node_view: m_selected_nodeview )
                    node_view->translate(delta, flags);
            }
            else
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                for (auto &node_view: get_all_nodeviews())
                    node_view->translate(delta, flags);
            }

            ImGui::ResetMouseDragDelta();
            break;
        }

        case ToolType_DEFINE_ROI:
        {
            // Update ROI second point
            m_tool.roi.end_pos = mouse_pos;
            // Get normalized ROI rectangle
            Rect roi = m_tool.roi.get_rect();
            // Expand to avoid null area
            const int roi_border_width = 2;
            roi.expand(Vec2{roi_border_width*0.5f});
            // Draw the ROI rectangle
            float alpha = wave(0.5f, 0.75f, (float) BaseApp::elapsed_time(), 10.f);
            draw_list->AddRect(roi.min, roi.max, ImColor(1.f, 1.f, 1.f, alpha), roi_border_width, ImDrawFlags_RoundCornersAll ,roi_border_width );
            break;
        }

        case ToolType_CREATE_WIRE: {
            ASSERT(m_tool.wire.dragged_slot.view != nullptr)
            if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
                break;
            if (!ImGui::IsMouseReleased(0))
                break;

            if (hovered.type == ItemType_NODEVIEW)
                ASSERT(false) // not handled, we only handle slotview, implement it?

            if (hovered.type != ItemType_SLOTVIEW) {
                ImGui::OpenPopup(k_CONTEXT_MENU_POPUP);
                break;
            }

            auto &event_manager = EventManager::get_instance();
            auto event = new Event_SlotDropped();
            event->data.first = &m_tool.wire.dragged_slot.view->get_slot();
            event->data.second = &hovered.slot.view->get_slot();
            event_manager.dispatch(event);

            break;
        }

        case ToolType_NONE:
            // nothing to do ...
            break;

        default:
            ASSERT(false) // unhandled case. Missing case for a new Tool?
    }

    // Context menu (draw)
    if (ImGui::BeginPopup(k_CONTEXT_MENU_POPUP))
    {
        bool draw_search_input = false;
        mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        switch (m_focused.type)
        {
            case ItemType_SLOTVIEW:
            {
                ASSERT(m_focused.slot.view != nullptr)

                // Disconnect focused SlotView?
                Slot *slot = &m_focused.slot.view->get_slot();
                bool can_disconnect = slot->empty();
                if (ImGui::MenuItem(ICON_FA_TRASH " Disconnect", nullptr, can_disconnect)) {
                    auto &event_manager = EventManager::get_instance();
                    event_manager.dispatch<Event_SlotDisconnected>({slot});
                    ImGui::CloseCurrentPopup();
                }
                draw_search_input = true;
                break;
            }

            case ItemType_EDGE:
            {
                ASSERT(m_focused.edge.tail != nullptr)
                ASSERT(m_focused.edge.head != nullptr)

                // Delete focused Wire?
                if (ImGui::Button(ICON_FA_TRASH" Delete Edge"))
                {
                    LOG_MESSAGE("GraphView", "Delete Edge Button clicked!\n");
                    // Generate an event from this action, add some info to the state and dispatch it.
                    auto &event_manager = EventManager::get_instance();
                    event_manager.dispatch<Event_DeleteEdge>({&m_focused.edge.tail->get_slot(), &m_focused.edge.head->get_slot()});
                    ImGui::CloseCurrentPopup();
                }
                draw_search_input = true;
                break;
            }

            case ItemType_NODEVIEW:
            {
                ASSERT(m_focused.node.view != nullptr)

                if (ImGui::MenuItem("Arrange"))
                    m_focused.node.view->arrange_recursively();

                ImGui::MenuItem("Pinned", "", &m_focused.node.view->m_pinned, true);

                if (ImGui::MenuItem("Expanded", "", &m_focused.node.view->m_expanded, true))
                    m_focused.node.view->set_expanded(m_focused.node.view->m_expanded);


                ImGui::Separator();

                auto flags = enable_edition ? ImGuiSelectableFlags_None
                                            : ImGuiSelectableFlags_Disabled;
                if (ImGui::Selectable("Delete", flags)) {
                    m_focused.node.view->get_node()->flagged_to_delete = true;
                }
                break;
            }

            default:
                ASSERT(false) // Unhandled case, is it a new one?

            case ItemType_NONE:
                draw_search_input = true;
                break;
        }

        // Draw the Node Search Input (advanced search menu)
        if (draw_search_input)
        {
            if( !m_context_menu_open_last_frame )
            {
                m_create_node_menu.reset_state();
            }
            ImGuiEx::ColoredShadowedText(Vec2(1, 1), Color(0, 0, 0, 255), Color(255, 255, 255, 127),
                                         "Create new node :");
            ImGui::Separator();
            SlotView *slotview = nullptr;
            if (m_tool.type == ToolType_CREATE_WIRE)
            {
                ASSERT(m_tool.wire.dragged_slot.view)
                slotview = m_tool.wire.dragged_slot.view;
            }
            if (Action_CreateNode *triggered_action = m_create_node_menu.draw_search_input(slotview, 10))
            {
                // Generate an event from this action, add some info to the state and dispatch it.
                auto &event_manager = EventManager::get_instance();
                auto *event = triggered_action->make_event();
                event->data.graph = m_graph;
                event->data.active_slotview = slotview;
                event->data.desired_screen_pos = mouse_pos_snapped;
                event_manager.dispatch(event);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
        m_context_menu_open_last_frame = true;
    }
    else
    {
        if (m_context_menu_open_last_frame && m_tool.type != ToolType_NONE)
            reset_tool();
        m_context_menu_open_last_frame = false;
    }

    // Tool Post-Update
    // Here we can change tool without breaking anything.
    switch( m_tool.type )
    {
        case ToolType_NONE:
        {
            switch (hovered.type)
            {
                case ItemType_SLOTVIEW:
                {
                    ASSERT(hovered.slot.view != nullptr)
                    if (ImGui::IsMouseDragging(0, 0.1f))
                        change_tool(DrawWire_Tool{hovered.slot.view});
                    else if (ImGui::IsMouseReleased(2))
                        m_focused = hovered.slot;
                    break;
                }

                case ItemType_NODEVIEW: {
                    ASSERT(hovered.node.view != nullptr)

                    if (ImGui::IsMouseClicked(0) && !hovered.node.view->selected)
                    {
                        // Add/Replace selection?
                        bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                                               ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                        SelectionMode flags = control_pressed ? SelectionMode_ADD
                                                              : SelectionMode_REPLACE;
                        set_selected({hovered.node.view}, flags);
                        m_focused = NodeView_Item{hovered.node.view};
                    }
                    else if (ImGui::IsMouseDoubleClicked(0))
                    {
                        // Expand/Collapse
                        hovered.node.view->expand_toggle();
                        m_focused = NodeView_Item{hovered.node.view};
                    }
                    else if (ImGui::IsMouseDragging(0) )
                    {
                        // Drag current selection
                        change_tool( DragNodeViews_Tool{DragNodeViews_Tool::Mode::SELECTION} );
                    }
                    break;
                }

                case ItemType_POSITION:
                    ASSERT(false) // not handled

                case ItemType_EDGE:
                {
                    if (ImGui::IsMouseDragging(0, 0.1f) )
                        m_focused = hovered;
                    else if ( ImGui::IsMouseClicked(1) )
                    {
                        m_focused = hovered;
                        ImGui::OpenPopup(k_CONTEXT_MENU_POPUP);
                    }
                    break;
                }

                case ItemType_NONE:
                {
                    if (ImGui::IsMouseDragging(0, 0.1f) && m_focused.type != ItemType_EDGE )
                    {
                        // Drag (Selection OR region of interest)
                        if (ImGui::IsKeyDown(ImGuiKey_Space))
                            change_tool( DragNodeViews_Tool{} );
                        else
                            change_tool( ROI_Tool{mouse_pos} );
                    }
                    else if (ImGui::IsMouseClicked(0))
                    {
                        // Deselect All (Click on the background)
                        set_selected({}, SelectionMode_REPLACE);
                    }
                    else if (ImGui::IsMouseReleased(1))
                    {
                        // Open Context Menu
                        m_focused = {};
                        ImGui::OpenPopup(k_CONTEXT_MENU_POPUP);
                    }
                    break;
                }

                default:
                    ASSERT(false) // unhandled case. Missing case for a new Tool?
            }
            break;
        }

        case ToolType_DEFINE_ROI:
        {
            if (ImGui::IsMouseReleased(0))
            {
                // Select the views included in the ROI
                NodeViewVec nodeview_in_roi;
                for (NodeView* nodeview: get_all_nodeviews())
                    if (Rect::contains( m_tool.roi.get_rect(), nodeview->get_rect(SCREEN_SPACE)))
                        nodeview_in_roi.emplace_back(nodeview);

                bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                                       ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                SelectionMode flags = control_pressed ? SelectionMode_ADD
                                                      : SelectionMode_REPLACE;
                set_selected(nodeview_in_roi, flags);

                // Switch back to default tool
                reset_tool();
            }
            break;
        }

        case ToolType_CREATE_WIRE:
        {
            // Draw temp wire
            draw_wire_from_slot_to_pos(m_tool.wire.dragged_slot.view , mouse_pos_snapped );
            break;
        }

        case ToolType_DRAG:
            if ( !ImGui::IsMouseDragging(0) )
                reset_tool();
            break;
    }

    if ( hovered.type == ItemType_NONE && ImGui::IsMouseReleased(0))
        m_focused = {};

    // add some empty space
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 100.0f);

    // Debug Infos
    if ( cfg->tools_cfg->runtime_debug )
    {
        if ( ImGui::Begin("GraphView debug info" ) )
        {
            ImGui::Text("m_focused.type: %i", m_focused.type);
            ImGui::Text("m_tool.type:    %i", m_tool.type);
            ImGui::Text("hovered.type:   %i", hovered.type);
            ImGui::Text("mouse_pos:          (%f, %f)", mouse_pos.x, mouse_pos.y);
            ImGui::Text("mouse_pos_snapped:  (%f, %f)", mouse_pos_snapped.x, mouse_pos_snapped.y);
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
            EXPECT(false, "unhandled case!")
    }
}

Action_CreateNode* CreateNodeContextMenu::draw_search_input(SlotView* dragged_slot, size_t _result_max_count )
{
    if ( must_be_reset_flag )
    {
        ImGui::SetKeyboardFocusHere();

        //
        update_cache_based_on_signature(dragged_slot);

        // Initial search
        update_cache_based_on_user_input(dragged_slot, 100 );

        // Ensure we reset once
        must_be_reset_flag = false;
    }

    // Draw search input and update_cache_based_on_user_input on input change
    if ( ImGui::InputText("Search", search_input, 255, ImGuiInputTextFlags_EscapeClearsAll ))
    {
        update_cache_based_on_user_input(dragged_slot, 100 );
    }

    if ( !items_matching_search.empty() )
    {
        // When a single item is filtered, pressing enter will press the item's button.
        if ( items_matching_search.size() == 1)
        {
            auto action = items_matching_search.front();
            if ( ImGui::SmallButton( action->label.c_str()) || ImGui::IsKeyDown( ImGuiKey_Enter ) )
            {
                return action;
            }
        }
        else
        {
            size_t more = items_matching_search.size() > _result_max_count ? items_matching_search.size() : 0;
            if ( more )
            {
                ImGui::Text("Found %zu result(s)", items_matching_search.size() );
            }
            // Otherwise, user has to move with arrow keys and press enter to trigger the highlighted button.
            auto it = items_matching_search.begin();
            while( it != items_matching_search.end() && std::distance(items_matching_search.begin(), it) != _result_max_count)
            {
                auto* action = *it;

                // User can click on the button...
                ImGui::Button( action->label.c_str());
                if( ImGui::IsItemClicked(0) )
                    return action;

                // ...or press enter if this item is the first
                if ( ImGui::IsKeyDown( ImGuiKey_Enter ) && ImGui::IsItemFocused() )
                    return action;

                it++;
            }
            if ( more )
            {
                ImGui::Text(".. %zu more ..", more );
            }
        }
    }
    else
    {
        ImGui::Text("No matches...");
    }

    return nullptr;
}
void CreateNodeContextMenu::update_cache_based_on_signature(SlotView* dragged_slot)
{
    items_with_compatible_signature.clear();

    // 1) When NO slot is dragged
    //---------------------------

    if ( !dragged_slot )
    {
        // When no slot is dragged, user can create any node
        items_with_compatible_signature = items;
        return;
    }

    // 2) When a slot is dragged
    //--------------------------

    for (auto& action: items )
    {
        const type* dragged_property_type = dragged_slot->get_property_type();

        switch ( action->event_data.node_type )
        {
            case NodeType_BLOCK_CONDITION:
            case NodeType_BLOCK_FOR_LOOP:
            case NodeType_BLOCK_WHILE_LOOP:
            case NodeType_BLOCK_SCOPE:
            case NodeType_BLOCK_PROGRAM:
                // Blocks are only for code flow slots
                if ( !dragged_slot->allows(SlotFlag_TYPE_CODEFLOW) )
                    continue;
                break;

            default:

                if ( dragged_slot->allows(SlotFlag_TYPE_CODEFLOW))
                {
                    // we can connect anything to a code flow slot
                }
                else if ( dragged_slot->allows(SlotFlag_INPUT) && dragged_slot->get_property_type()->is<Node*>() )
                {
                    // we can connect anything to a Node ref input
                }
                else if ( action->event_data.node_signature )
                {
                    // discard incompatible signatures

                    if ( dragged_slot->allows( SlotFlag_ORDER_FIRST ) &&
                         !action->event_data.node_signature->has_an_arg_of_type(dragged_property_type)
                       )
                        continue;

                    if ( !action->event_data.node_signature->get_return_type()->equals(dragged_property_type) )
                        continue;

                }
        }
        items_with_compatible_signature.push_back( action );
    }
}

void CreateNodeContextMenu::update_cache_based_on_user_input(SlotView* _dragged_slot, size_t _limit )
{
    items_matching_search.clear();
    for ( auto& menu_item : items_with_compatible_signature )
    {
        if( menu_item->label.find( search_input ) != std::string::npos )
        {
            items_matching_search.push_back(menu_item);
            if ( items_matching_search.size() == _limit )
            {
                break;
            }
        }
    }
}

void CreateNodeContextMenu::reset_state()
{
    must_be_reset_flag   = true;
    search_input[0]      = '\0';
    items_matching_search.clear();
    items_with_compatible_signature.clear();
}

void CreateNodeContextMenu::add_action(Action_CreateNode* action)
{
    items.push_back(action);
}

void GraphView::set_selected(const NodeViewVec& views, SelectionMode mode )
{
    NodeViewVec curr_selection = m_selected_nodeview;
    if ( mode == SelectionMode_REPLACE )
    {
        m_selected_nodeview.clear();
        for(auto& each : curr_selection )
            each->selected = false;
    }

    for(auto& each : views)
    {
        m_selected_nodeview.emplace_back(each);
        each->selected = true;
    }

    EventPayload_NodeViewSelectionChange event{ m_selected_nodeview, curr_selection };
    EventManager& event_manager = EventManager::get_instance();
    event_manager.dispatch<Event_SelectionChange>(event);
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

void GraphView::change_tool(Tool new_tool)
{
    // Note: if this start to grow considerably, consider using a State Machine

    EXPECT(new_tool.type != m_tool.type, "Cannot set same new_tool twice, set none new_tool first.")

    // Initialize GraphView state depending on the new tool
    switch (new_tool.type)
    {
        case ToolType_DRAG:
        {
            for(auto& each : m_selected_nodeview)
            each->m_pinned = true;
            break;
        }
        case ToolType_NONE:
        case ToolType_CREATE_WIRE:
        case ToolType_DEFINE_ROI:
            // nothing...
            break;
        default:
            ASSERT(false) // not handled, is it a new missing case?
    }

    m_tool = new_tool;
}

bool GraphView::has_an_active_tool() const
{
    return m_tool.type != ToolType_NONE;
}

void GraphView::reset_all_properties()
{
    for( NodeView* each : get_all_nodeviews() )
        for( PropertyView* property_view : each->m_property_views )
            property_view->reset();
}

void GraphView::reset_tool()
{
    change_tool(Tool{});
}
