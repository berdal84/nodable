#include "GraphViewTool.h"
#include "GraphView.h"
#include "tools/gui/BaseApp.h"
#include "tools/core/math.h"
#include "imgui.h"
#include "tools/core/Color.h"

using namespace ndbl;
using namespace tools;

void Tool::tick()
{
    State old_state = state;

    // Update focused item
    if ( context.hovered.type == ItemType_NONE && ImGui::IsMouseReleased(0))
        context.focused = {};

    // Here we must not change tool, use Post-Update instead.
    switch ( state.type )
    {
        case ToolType_DRAG:
        {
            Vec2 delta = ImGui::GetMouseDragDelta();
            if (delta.lensqr() < 1) // avoid updating when mouse is static
                break;

            NodeViewFlags flags = NodeViewFlag_IGNORE_SELECTED | NodeViewFlag_IGNORE_PINNED;
            if (state.drag.mode == DragTool::Mode::SELECTION)
            {
                for (auto &node_view: context.graph_view->get_selected() )
                    node_view->translate(delta, flags);
            }
            else
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                for (auto &node_view: context.graph_view->get_all_nodeviews())
                    node_view->translate(delta, flags);
            }

            ImGui::ResetMouseDragDelta();
            break;
        }

        case ToolType_ROI:
        {
            // Update ROI second point
            state.roi.end_pos = context.mouse_pos;
            // Get normalized ROI rectangle
            Rect roi = state.roi.get_rect();
            // Expand to avoid null area
            const int roi_border_width = 2;
            roi.expand(Vec2{roi_border_width*0.5f});
            // Draw the ROI rectangle
            float alpha = wave(0.5f, 0.75f, (float) BaseApp::elapsed_time(), 10.f);
            context.draw_list->AddRect(roi.min, roi.max, ImColor(1.f, 1.f, 1.f, alpha), roi_border_width, ImDrawFlags_RoundCornersAll ,roi_border_width );
            break;
        }

        case ToolType_LINE: {
            ASSERT(state.wire.dragged_slot.view != nullptr)
            if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
                break;
            if (!ImGui::IsMouseReleased(0))
                break;

            if (context.hovered.type == ItemType_NODEVIEW)
                ASSERT(false) // not handled, we only handle slotview, implement it?

            if (context.hovered.type != ItemType_SLOTVIEW) {
                ImGui::OpenPopup(k_CONTEXT_MENU_POPUP);
                break;
            }

            auto &event_manager = EventManager::get_instance();
            auto event = new Event_SlotDropped();
            event->data.first = &state.wire.dragged_slot.view->get_slot();
            event->data.second = &context.hovered.slot.view->get_slot();
            event_manager.dispatch(event);

            break;
        }

        case ToolType_CURSOR:
            // nothing to do ...
            break;

        default:
            ASSERT(false) // unhandled case. Missing case for a new Tool?
    }
    EXPECT(state.type == old_state.type, "changing state above this line is forbidden")
}

void Tool::draw()
{
    State old_state = state;

    // Context menu (draw)
    if (ImGui::BeginPopup(k_CONTEXT_MENU_POPUP))
    {
        bool draw_search_input = false;
        context.mouse_pos_snapped = ImGui::GetMousePosOnOpeningCurrentPopup();

        switch (context.focused.type)
        {
            case ItemType_SLOTVIEW:
            {
                ASSERT(context.focused.slot.view != nullptr)

                // Disconnect focused SlotView?
                Slot *slot = &context.focused.slot.view->get_slot();
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
                ASSERT(context.focused.edge.tail != nullptr)
                ASSERT(context.focused.edge.head != nullptr)

                // Delete focused Wire?
                if (ImGui::Button(ICON_FA_TRASH" Delete Edge"))
                {
                    LOG_MESSAGE("GraphView", "Delete Edge Button clicked!\n");
                    // Generate an event from this action, add some info to the state and dispatch it.
                    auto &event_manager = EventManager::get_instance();
                    event_manager.dispatch<Event_DeleteEdge>({&context.focused.edge.tail->get_slot(), &context.focused.edge.head->get_slot()});
                    ImGui::CloseCurrentPopup();
                }
                draw_search_input = true;
                break;
            }

            case ItemType_NODEVIEW:
            {
                ASSERT(context.focused.node.view != nullptr)

                if (ImGui::MenuItem("Arrange"))
                    context.focused.node.view->arrange_recursively();

                ImGui::MenuItem("Pinned", "", context.focused.node.view->pinned(), true);

                if (ImGui::MenuItem("Expanded", "", context.focused.node.view->is_expanded(), true))
                    context.focused.node.view->set_expanded(context.focused.node.view->is_expanded());

                ImGui::Separator();

                if (ImGui::Selectable("Delete")) {
                    context.focused.node.view->get_node()->flagged_to_delete = true;
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
            if( !context.cxt_menu_open_last_frame )
            {
                context.create_node_ctx_menu.reset_state();
            }
            ImGuiEx::ColoredShadowedText(Vec2(1, 1), Color(0, 0, 0, 255), Color(255, 255, 255, 127),
                                         "Create new node :");
            ImGui::Separator();
            SlotView *slotview = nullptr;
            if (state.type == ToolType_LINE)
            {
                ASSERT(state.wire.dragged_slot.view)
                slotview = state.wire.dragged_slot.view;
            }
            if (Action_CreateNode *triggered_action = context.create_node_ctx_menu.draw_search_input(slotview, 10))
            {
                // Generate an event from this action, add some info to the state and dispatch it.
                auto& event_manager            = EventManager::get_instance();
                auto event                     = triggered_action->make_event();
                event->data.graph              = context.graph_view->get_graph();
                event->data.active_slotview    = slotview;
                event->data.desired_screen_pos = context.mouse_pos_snapped;
                event_manager.dispatch(event);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
        context.cxt_menu_open_last_frame = true;
        EXPECT(state.type == old_state.type, "changing state above this line is forbidden")
    }
    else
    {
        if (context.cxt_menu_open_last_frame && state.type != ToolType_CURSOR)
            reset_state();
        context.cxt_menu_open_last_frame = false;
    }

    check_state();
}

void Tool::check_state()
{
    // Here we can change tool without breaking anything.
    switch( state.type )
    {
        case ToolType_CURSOR:
        {
            switch (context.hovered.type)
            {
                case ItemType_SLOTVIEW:
                {
                    ASSERT(context.hovered.slot.view != nullptr)
                    if (ImGui::IsMouseDragging(0, 0.1f))
                    {
                        State new_state;
                        new_state.wire = {context.hovered.slot.view};
                        return change_state(new_state);
                    }
                    else if (ImGui::IsMouseReleased(2))
                        context.focused = context.hovered.slot;
                    break;
                }

                case ItemType_NODEVIEW:
                {
                    ASSERT(context.hovered.node.view != nullptr)

                    if (ImGui::IsMouseClicked(0) && !context.hovered.node.view->selected)
                    {
                        // Add/Replace selection?
                        bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                                               ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                        SelectionMode flags = control_pressed ? SelectionMode_ADD
                                                              : SelectionMode_REPLACE;
                        context.graph_view->set_selected({context.hovered.node.view}, flags);
                        context.focused = NodeViewItem{context.hovered.node.view};
                    }
                    else if (ImGui::IsMouseDoubleClicked(0))
                    {
                        // Expand/Collapse
                        context.hovered.node.view->expand_toggle();
                        context.focused = NodeViewItem{context.hovered.node.view};
                    }
                    else if (ImGui::IsMouseDragging(0) )
                    {
                        State new_state;
                        new_state.drag = {DragTool::Mode::SELECTION};
                        return change_state(new_state);
                    }
                    break;
                }

                case ItemType_POSITION:
                    ASSERT(false) // not handled

                case ItemType_EDGE:
                {
                    if (ImGui::IsMouseDragging(0, 0.1f) )
                        context.focused = context.hovered;
                    else if ( ImGui::IsMouseClicked(1) )
                    {
                        context.focused = context.hovered;
                        ImGui::OpenPopup(k_CONTEXT_MENU_POPUP);
                    }
                    break;
                }

                case ItemType_NONE:
                {
                    if (ImGui::IsMouseDragging(0, 0.1f) && context.focused.type != ItemType_EDGE )
                    {
                        State new_state;
                        // Drag (Selection OR region of interest)
                        if (ImGui::IsKeyDown(ImGuiKey_Space))
                            new_state.drag = {};
                        else
                            new_state.roi = {context.mouse_pos};
                        return change_state( new_state );
                    }
                    else if (ImGui::IsMouseClicked(0))
                    {
                        // Deselect All (Click on the background)
                        context.graph_view->set_selected({}, SelectionMode_REPLACE);
                    }
                    else if (ImGui::IsMouseReleased(1))
                    {
                        // Open Context Menu
                        context.focused = {};
                        ImGui::OpenPopup(k_CONTEXT_MENU_POPUP);
                    }
                    break;
                }

                default:
                    ASSERT(false) // unhandled case. Missing case for a new Tool?
            }
            break;
        }

        case ToolType_ROI:
        {
            if (ImGui::IsMouseReleased(0))
            {
                // Select the views included in the ROI
                std::vector<NodeView*> nodeview_in_roi;
                for (NodeView* nodeview: context.graph_view->get_all_nodeviews())
                    if (Rect::contains( state.roi.get_rect(), nodeview->get_rect(SCREEN_SPACE)))
                        nodeview_in_roi.emplace_back(nodeview);

                bool control_pressed = ImGui::IsKeyDown(ImGuiKey_LeftCtrl) ||
                                       ImGui::IsKeyDown(ImGuiKey_RightCtrl);
                SelectionMode flags = control_pressed ? SelectionMode_ADD
                                                      : SelectionMode_REPLACE;
                context.graph_view->set_selected(nodeview_in_roi, flags);

                // Switch back to default tool
                return reset_state();
            }
            break;
        }

        case ToolType_LINE:
        {
            // Draw temp wire
            context.graph_view->draw_wire_from_slot_to_pos(state.wire.dragged_slot.view , context.mouse_pos_snapped );
            break;
        }

        case ToolType_DRAG:
            if ( !ImGui::IsMouseDragging(0) )
                reset_state();
            break;
    }
}


ToolType Tool::tool_type() const
{
    return state.type;
}

void Tool::change_state(const State& new_state)
{
    // Initialize GraphView state depending on the new tool
    switch (new_state.type)
    {
        case ToolType_DRAG:
        {
            for(auto& each : context.selected_nodeview)
                each->set_pinned(true);
            break;
        }
        case ToolType_CURSOR:
        case ToolType_LINE:
        case ToolType_ROI:
            // nothing...
            break;
        default:
            ASSERT(false) // not handled, is it a new missing case?
    }

    state = new_state;
}

void Tool::reset_state()
{
    change_state(State{});
}
