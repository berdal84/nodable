#include "Node_View.h"

#include <algorithm> // for std::max
#include <glm/trigonometric.hpp> // for sinus
#include <vector>

#include "bdc/Allocators.hpp"
#include "tools/core/Asserts.h"
#include "tools/gui/ImGuiEx.h"
#include "tools/gui/View_Flags.h"
#include "tools/gui/geometry/Pivots.h"
#include "tools/gui/geometry/Space.h"
#include "tools/core/Math.h"
#include "tools/core/Flags.h"

#include "ndbl/core/Graph.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/Scope.h"

#include "ndbl/gui/Scope_View.h"
#include "ndbl/gui/Config.h"
#include "ndbl/gui/Node_Property_View.h"
#include "ndbl/gui/Node_Slot_View.h"
#include "ndbl/gui/Graph_View.h"

#ifdef NDBL_DEBUG
#define NDBL_ASTNODEVIEW_DEBUG_DRAW 0
#endif

using namespace ndbl;
using namespace tools;

#define PIXEL_PERFECT true // round positions for drawing only


void ndbl::nodeview_init(Node_View* nodeview, Node* node)
{
    assert(node);    
    nodeview->node = node;

    Config* cfg = config();

    // 1. Create Property views
    //-------------------------

    VERIFY(nodeview->view_by_property.empty(), "Cannot be called twice");

    for (Node_Property* property : nodeview->node->props )
    {
        // Create view
        auto new_view = bdc::memory_new<Node_Property_View>();
        nodepropertyview_init(new_view, property);
        spatialnode_add_child(&nodeview->shape.spatial_node, &new_view->shape.spatial_node);
        spatialnode_set_position(&new_view->shape.spatial_node, {}, tools::PARENT_SPACE);

        switch ( nodeview->node->type )
        {
            case Node_Type_ROOT:
            case Node_Type_SCOPE:
            case Node_Type_FUNCTION:
            case Node_Type_OPERATOR:
            case Node_Type_FOR_LOOP:
            case Node_Type_IF_ELSE:
            case Node_Type_WHILE_LOOP:
            {
                // hide THIS property
                if ( HAS_FLAGS(property->flags, Node_Property::Flag_IS_NODE_VALUE) )
                {
                    SET_FLAGS(new_view->flags, View_Flag_HIDDEN);
                }
            }
        }

        // Indexing
        if (property == nodeview->node->value )
        {
            nodeview->value_view = new_view;
        }

        bool has_in  = node_find_slot_by_property(nodeview->node, property, Node_Slot::Flag_INPUT );
        bool has_out = node_find_slot_by_property(nodeview->node, property, Node_Slot::Flag_OUTPUT );

        if ( has_in)
            nodeview->view_by_property_type[Property_Category_IN].push_back(new_view);
        if ( has_out)
            nodeview->view_by_property_type[Property_Category_OUT].push_back(new_view);

        if ( has_in && has_out )
            nodeview->view_by_property_type[Property_Category_INOUT_STRICTLY].push_back(new_view);
        else if ( has_in )
            nodeview->view_by_property_type[Property_Category_IN_STRICTLY].push_back(new_view);
        else if ( has_out )
            nodeview->view_by_property_type[Property_Category_OUT_STRICTLY].push_back(new_view);

        nodeview->view_by_property.emplace(property, new_view);
    }

    // 2. Create a Node_Slot_View per slot
    //------------------------------

    for(Node_Slot_View* each : nodeview->slot_views )
        bdc::memory_delete(each);
    nodeview->slot_views.clear();

    auto get_shapetype = [](const Node_Slot* slot)
    {
        switch ( slot->flags & Node_Slot::Flag_TYPE_MASK )
        {
            case Node_Slot::Flag_TYPE_FLOW:
                return Shape_Type_RECTANGLE;
            case Node_Slot::Flag_TYPE_VALUE:
                return Shape_Type_CIRCLE;
            default:
                ASSERT(false); // no implemented yet
                return Shape_Type_CIRCLE;
        }
    };

    auto get_pivot = [](const Node_Slot* slot)
    {
        switch( slot->flags & ( Node_Slot::Flag_TYPE_MASK | Node_Slot::Flag_ORDER_MASK ) )
        {
            case Node_Slot::Flag_INPUT:
                return TOP;
            case Node_Slot::Flag_OUTPUT:
                return BOTTOM;
            case Node_Slot::Flag_FLOW_IN:
                return TOP_LEFT;
            case Node_Slot::Flag_FLOW_OUT:
                return BOTTOM_LEFT;
            default:
                ASSERT(false); // not implemented yet
                return TOP;
        }
    };

    std::unordered_map<Node_Slot::Flags, u8_t> count_per_type
    {
        {Node_Slot::Flag_FLOW_OUT, 0 },
        {Node_Slot::Flag_FLOW_IN , 0 },
        {Node_Slot::Flag_INPUT   , 0 },
        {Node_Slot::Flag_OUTPUT  , 0 }
    };

    // Create a view per slot
    for( Node_Slot* slot : nodeview->node->slots )
    {
        const u8_t index = count_per_type.at(slot->type_and_order())++;
        auto* view = bdc::memory_new<Node_Slot_View>();
        nodeslotview_init(view, slot, get_pivot(slot), get_shapetype(slot), index, &nodeview->shape);
        spatialnode_add_child( &nodeview->shape.spatial_node, &view->shape.spatial_node );
        spatialnode_set_position(&view->shape.spatial_node, {}, tools::PARENT_SPACE);
        
        nodeview->slot_views.push_back(view);
    }

    // Make sure inputs/outputs are aligned with the property views (if present) and not the node's view.
    for(Node_Slot_View* slot_view : nodeview->slot_views)
    {
        switch ( slot_view->slot->type() )
        {
            case Node_Slot::Flag_TYPE_VALUE:
            {
                const Node_Property_View* property_view = nodeview_find_property_view(nodeview, slot_view->property());
                if ( property_view != nullptr && !HAS_FLAGS(property_view->flags, View_Flag_HIDDEN) )
                    slot_view->alignment_ref = &property_view->shape;
            }
        }
    }

    // Adjust some slot views
    switch ( nodeview->node->type )
    {
        case Node_Type_VARIABLE:
        {
            if ( Node_Slot* decl_out =  nodeview->node->component.variable.decl_out )
            {
                if (Node_Slot_View *view = decl_out->view)
                {
                    view->alignment_pivot = LEFT;
                    nodeslotview_update_direction_from_alignment(view);
                    view->alignment_ref = &nodeview->shape;
                }
            }
            break;
        }
        case Node_Type_FUNCTION:
        {
            if ( Node_Slot* value_out = nodeview->node->value_out() )
            {
                if (Node_Slot_View *view = value_out->view)
                {
                    view->direction     = BOTTOM;
                    view->alignment_ref = nullptr;
                }
            }
            break;
        }
    }

    // 3. Update fill color
    //---------------------

    // note: We pass color by address to be able to change the color dynamically
    nodeview->colors[Color_FILL] = &cfg->ui_node_fill_color[ nodeview->node->type];

    // 4. Create Scope_View
    //--------------------

    if ( Scope* internal_scope = nodeview->node->internal_scope )
    {
        auto* scopeview = bdc::memory_new<Scope_View>();
        scopeview_init(scopeview, internal_scope);
        internal_scope->view         = scopeview;
        #warning TODO: remove this duplicated state, we could get the internal_scopeview with a getter (having nullptr conditions)
        nodeview->internal_scopeview = scopeview;
    }
}

void ndbl::nodeview_deinit(Node_View* nodeview)
{
    spatialnode_clear(&nodeview->shape.spatial_node);

    for(auto& [_, each] : nodeview->view_by_property )
        bdc::memory_delete(each);
    nodeview->view_by_property.clear();

    for(auto& vector : nodeview->view_by_property_type )
        vector.clear();
    // no m_view_by_property_type.clear(), it is an array ;)

    for(auto* each : nodeview->slot_views )
        bdc::memory_delete(each);
    nodeview->slot_views.clear();

    if( nodeview->internal_scopeview != nullptr )
    {
        scopeview_deinit(nodeview->internal_scopeview);
    }
    nodeview->hovered_slotview = nullptr;
}

bdc::String ndbl::nodeview_get_label(const Node_View* nodeview)
{
    Config* cfg = config();

    bool minimalist = cfg->ui_node_detail == View_Detail_COMPACT;

    switch (nodeview->node->type )
    {
        case Node_Type_VARIABLE_REF:
        {
            if ( minimalist )
                return "&";
            return nodeview->node->name;
        }
        case Node_Type_VARIABLE:
        {
            if (minimalist)
                return "";
            return node_variable_type(nodeview->node)->name;
        }
        case Node_Type_OPERATOR:
        {
            return nodeview->node->name;
        }
        case Node_Type_FUNCTION:
        {
            if ( minimalist )
                return "f(x)";
            return nodeview->node->name;
        }
        case Node_Type_ROOT:
        case Node_Type_SCOPE:
        {
            if ( minimalist )
            {
                return string_lsplit(nodeview->node->name, 6); // 4 char for the icon
            }
            return nodeview->node->name;
        }
        case Node_Type_IF_ELSE:
        {
            if ( minimalist )
                return "?";
            return nodeview->node->name;
        }
        case Node_Type_FOR_LOOP:
        {
            if ( minimalist )
                return "for";
            return nodeview->node->name;
        }
        default:
        {
            if ( minimalist )
                return string_lsplit(nodeview->node->name, 3);
            return nodeview->node->name;
        }
    }

}

void ndbl::nodeview_arrange_recursively(Node_View* nodeview, bool _smoothly)
{
    for (auto each_input: nodeview_get_adjacent(nodeview, Node_Slot::Flag_INPUT) )
    {
        if ( !HAS_FLAGS(each_input->flags, View_Flag_PINNED) )
            if (node_is_output_node_in_expression(each_input->node, nodeview->node ) )
                nodeview_arrange_recursively(each_input);
    }

    if (Scope* internal_scope = nodeview->node->internal_scope )
        for ( Node* node : scope_get_backbone(internal_scope) )
            if ( node->view )
                    nodeview_arrange_recursively( node->view );

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well-placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        nodeview_update(nodeview, float(1000));
    }

    UNSET_FLAGS(nodeview->flags, View_Flag_PINNED);
}

void ndbl::nodeview_update(Node_View* nodeview, float dt)
{
    if( nodeview->opacity != 1.0f)
        tools::clamped_lerp(nodeview->opacity, 1.0f, 10.0f * dt);

    for(Node_Slot_View* each_slot_view  : nodeview->slot_views )
        nodeslotview_update( each_slot_view, dt );

    if ( nodeview->internal_scopeview != nullptr )
        scopeview_update( nodeview->internal_scopeview, dt );
}

bool ndbl::nodeview_draw(Node_View* nodeview)
{
    box2d_draw_debug_info(&nodeview->shape);

    if ( HAS_FLAGS(nodeview->flags, View_Flag_HIDDEN) )
    {
        return false;
    }

    ASSERT( nodeview->node );

    Config*     cfg       = config();
	bool        changed   = false;

    nodeview->hovered_slotview = nullptr; // reset every frame

    // Draw background slots (rectangles)
    for( Node_Slot_View* slot_view : nodeview->slot_views )
        if ( slot_view->shape_type == Shape_Type_RECTANGLE)
            nodeview_draw_slot(nodeview, slot_view);

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, nodeview->opacity);
    Rect screen_rect = nodeview->shape.rect(WORLD_SPACE);

#if PIXEL_PERFECT
        screen_rect.min.round();
        screen_rect.max.round();
#endif

    ImGui::SetCursorScreenPos( screen_rect.top_left() ); // start from th top left corner
	ImGui::PushID(nodeview);


	// Draw the background of the Group
    Vec4 border_color = cfg->ui_node_borderColor;
    if ( HAS_FLAGS(nodeview->flags, View_Flag_SELECTED) )
    {
        border_color = cfg->ui_node_borderHighlightedColor;
    }
    else if ( node_is_instruction( nodeview->node ) )
    {
        border_color = cfg->ui_node_fill_color[Node_Type_NULL];
    }

    float border_width = cfg->ui_node_borderWidth;
    if( node_is_instruction( nodeview->node ) )
    {
        border_width *= cfg->ui_node_instructionBorderRatio;
    }

    nodeview_draw_node_rect(
            screen_rect,
            *nodeview->colors[Color_FILL],
            cfg->ui_node_borderColor,
            cfg->ui_node_shadowColor,
            border_color,
            cfg->ui_node_border_radius,
            border_width );

    if ( HAS_FLAGS(nodeview->flags, View_Flag_SELECTED) )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        auto alpha   = glm::sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
        const float offset = cfg->ui_node_selected_rectangle_offset;
        draw_list->AddRect(
            screen_rect.min - Vec2(offset),
            screen_rect.max + Vec2(offset),
            ImColor(1.f, 1.f, 1.f, float(alpha) ),
            cfg->ui_node_border_radius + offset,
            ~0,
            offset / 2.0f
        );
    }

    bool is_rect_hovered = !ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringRect(screen_rect.min, screen_rect.max);

	// Draw the window content
	//------------------------

    ImGui::SetCursorScreenPos( screen_rect.top_left() );
    ImGui::BeginGroup();
    ImGui::SetCursorPosX( ImGui::GetCursorPosX() + cfg->ui_node_padding.x );
    ImGui::SetCursorPosY( ImGui::GetCursorPosY() + cfg->ui_node_padding.y );
    ImGui::AlignTextToFramePadding(); // text and other elements will be well aligned
    ImGui::Dummy({1.f});

    // We currently don't need to see these property, unnecessary complexity
    // ImGui::SameLine(); draw_properties(m_property_views_index_index[OUT_STRICTLY]);

    bdc::String pre_label;
    std::vector<bdc::String> operator_label(1); // for binary (and ternary when implemented) operators
    bdc::String post_label;

    switch ( nodeview->node->type )
    {
        case Node_Type_OPERATOR:
            if ( node_is_unary_operator(nodeview->node ) )
                pre_label = nodeview_get_label(nodeview);
            else if ( node_is_binary_operator(nodeview->node ) )
                operator_label[0] = nodeview_get_label(nodeview);
            // else if (node->is_ternary_operator()
            break;
        default:
            pre_label = nodeview_get_label(nodeview);
            break;
        case Node_Type_FUNCTION:
            pre_label = nodeview_get_label(nodeview);

            if ( cfg->ui_node_detail != View_Detail_COMPACT)
            {
                pre_label  = bdc::string_printf( bdc::temp_allocator(), "%s(", pre_label.c_str() );
                post_label = ")";
            }
            break;
    }

    if ( !nodeview->is_expanded )
        pre_label = bdc::string_printf( bdc::temp_allocator(), "%s " ICON_FA_OBJECT_GROUP, pre_label.c_str() );

    // Draw the pre_label when necessary
    if ( !pre_label.empty() ) {
        ImGui::SameLine();
        ImGui::Text("%s", pre_label.c_str());

        // Update slot_view_out to be positioned below the pre_label

        if (nodeview->node->type == Node_Type_FUNCTION )
            if (Node_Slot *slot_out = nodeview->node->value_out())
                if (Node_Slot_View *slot_view_out = slot_out->view)
                {
                    const float x = ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x * 0.5f;
                    const float y = nodeview->shape.pivot_position(BOTTOM, WORLD_SPACE).y;
                    spatialnode_set_position(&slot_view_out->shape.spatial_node, {x, y}, WORLD_SPACE);
                    slot_view_out->direction = BOTTOM;
                }
    }

    // Draw the properties depending on node type
    if (nodeview->node->type != Node_Type_OPERATOR )
    {
        changed |= nodepropertyview_draw_all(nodeview->view_by_property_type[Property_Category_IN_STRICTLY]    , cfg->ui_node_detail);
        changed |= nodepropertyview_draw_all(nodeview->view_by_property_type[Property_Category_INOUT_STRICTLY] , cfg->ui_node_detail);
        changed |= nodepropertyview_draw_all(nodeview->view_by_property_type[Property_Category_OUT_STRICTLY]   , cfg->ui_node_detail);
    }
    else
    {
        size_t i = 0;
        for( Node_Property_View* property_view : nodeview->view_by_property_type[Property_Category_IN] )
        {
            ImGui::SameLine();
            changed |= nodepropertyview_draw( property_view, cfg->ui_node_detail );

            // draw inner label when necessary
            if ( i < operator_label.size() && !operator_label[i].empty() )
            {
                ImGui::SameLine(); ImGui::Text("%s", operator_label[i].c_str() );
            }
            ++i;
        }
    }

    if ( !post_label.empty() )
    {
        ImGui::SameLine(); ImGui::Text("%s", post_label.c_str());
    }


    ImGui::EndGroup();

    // Ends the Window
    //----------------

    // Update box's size according to item's rect
    Vec2 new_size = ImGui::GetItemRectSize();

    new_size += Vec2{ cfg->ui_node_padding.z, cfg->ui_node_padding.w}; // right and bottom padding
    new_size.x = std::max( 1.0f, new_size.x );
    new_size.y = std::max( 1.0f, new_size.y );

    nodeview->shape.set_size(Vec2::round(new_size));

    // Draw foreground slots (circles)
    for( Node_Slot_View* slot_view: nodeview->slot_views )
        if ( slot_view->shape_type == Shape_Type_CIRCLE)
            nodeview_draw_slot(nodeview, slot_view);

	ImGui::PopStyleVar();
	ImGui::PopID();

    if ( changed )
        SET_FLAGS(nodeview->node->flags, Node_Flag_IS_DIRTY );

    const bool hovered = is_rect_hovered || nodeview->hovered_slotview != nullptr;
    SET_FLAGS_VALUE(nodeview->flags, View_Flag_HOVERED, hovered );

	return changed;
}

void ndbl::nodeview_draw_node_rect(
    Rect rect,
    Vec4 color,
    Vec4 border_highlight_col,
    Vec4 shadow_col,
    Vec4 border_col,
    float border_radius,
    float border_width
)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw the rectangle under everything
    ImGuiEx::DrawRectShadow(rect.min, rect.max, border_radius, 4, Vec2(1.0f), shadow_col);
    ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
    draw_list->AddRectFilled(rect.min, rect.max, ImColor(color), border_radius, flags);
    draw_list->AddRect(rect.min + Vec2(1.0f), rect.max, ImColor(border_highlight_col), border_radius, flags, border_width);
    draw_list->AddRect(rect.min, rect.max, ImColor(border_col), border_radius, flags, border_width);
}

bool ndbl::nodeview_draw_as_properties_panel(Node_View* nodeview, bool* _show_advanced)
{
    bool changed = false;
    Node* node = nodeview->node;
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](Node_Property_View* _property_view) -> bool
    {
        Node_Property* property = _property_view->property;
        // label (<name> (<type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s): ",
                property->name.c_str(),
                property->type->name.c_str());

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("Source token:\n %s\n", property->token.json().c_str());
            ImGuiEx::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        return nodepropertyview_draw_input(_property_view, !_show_advanced, nullptr);
    };

    ImGui::Text("Name:  \"%s\"", node->name.c_str() );
    ImGui::Text("Class: \"%s\"", node->get_class()->name.c_str() );

    // Draw exposed input properties

    auto draw_properties = [&](const bdc::String title, const std::vector<Node_Property_View*>& views) -> bool
    {
        bool changed = false;
        ImGui::Text("%s:", title.c_str());
        ImGui::Separator();
        ImGui::Indent();
        if( views.empty() )
        {
            ImGui::Text("None.");
            ImGui::Separator();
        }
        else
        {
            for (auto& property_view : views )
            {
                changed |= draw_labeled_property_view( property_view );
                ImGui::Separator();
            }
        }
        ImGui::Unindent();
        return changed;
    };

    ImGui::Separator();
    changed |= draw_properties("Inputs(s)", nodeview->view_by_property_type[Property_Category_IN_STRICTLY]);
    changed |= draw_properties("In/Out(s)", nodeview->view_by_property_type[Property_Category_INOUT_STRICTLY]);
    ImGui::Separator();
    changed |= draw_properties("Output(s)", nodeview->view_by_property_type[Property_Category_OUT_STRICTLY]);

#ifdef NDBL_DEBUG

    ImGui::Separator();
    changed |= draw_labeled_property_view( nodeview->value_view );

    if( node->internal_scope )
    {
        ImGui::Separator();
        TreeNode_Scope("Scope", node->internal_scope);
        ImGui::TreePop();
    }

    ImGui::Separator();
    ImGui::Text("Node_Slots");
    ImGui::Separator();
    auto draw_node_list = [](const char *label, const std::vector<Node*> _nodes )
        {
            if( !ImGui::TreeNode(label) )
            {
                return;
            }

            if ( _nodes.empty() )
            {
                ImGui::BulletText( "None" );
            }

            for (const Node* each_node : _nodes )
            {
                ImGui::BulletText("- %s", each_node->name.c_str());
            }

            ImGui::TreePop();
        };
    draw_node_list("Inputs:"     , node->inputs() );
    draw_node_list("Outputs:"    , node->outputs() );
    draw_node_list("FlowInputs:" , node->flow_inputs() );
    draw_node_list("FlowOutputs:", node->flow_outputs() );
    ImGui::Separator();

    if( ImGui::TreeNode("Others") )
    {
        if (ImGui::BeginTable("table", 2))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("scope");
            ImGui::TableNextColumn();
            Scope* scope = node->scope;
            if (scope)
            {
                bdc::String label = bdc::string_printf( bdc::temp_allocator(), "%s %p (%s %p)", scope->name.c_str(), scope, scope->node->name.c_str(), scope->node);
                if ( ImGui::Button(label.c_str()) )
                {
                    ASSERT(node->graph->view);
                    view_selection_clear(&node->graph->view->selection);
                    view_selection_add(&node->graph->view->selection, scope->node->view );
                }
            }
            else
            {
                ImGui::Text("nullptr");
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("dirty");
            ImGui::TableNextColumn();
            ImGui::Text(HAS_FLAGS(node->flags, Node_Flag_IS_DIRTY) ? "yes" : "no");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("suffix token");
            ImGui::TableNextColumn();
            ImGui::Text("%s", node->suffix.json().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("can_be_instruction");
            ImGui::TableNextColumn();
            ImGui::Text("%i", node_could_be_instruction(node));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("is_instruction");
            ImGui::TableNextColumn();
            ImGui::Text("%i", node_is_instruction(node));

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
#endif // NDBL_DEBUG
    return changed;
}

Rect ndbl::nodeview_get_rect_ex(const Node_View* nodeview, tools::Space space, Node_View_Flags flags)
{
    if( (flags & Node_View_Flag_WITH_RECURSION) == 0 )
        return nodeview->shape.rect(space);

    Rect result;

    if ( !HAS_FLAGS(nodeview->flags, View_Flag_HIDDEN) )
    {
        result = nodeview->shape.rect(space);
    }

    // TEMPORARILY DISABLED: we need a flag here to disable this type of "INNER_RECURSION"
    // if ( Scope* _internal_scope = node()->internal_scope() )
    // {
    //      result = result.bounding_rect(result, _internal_scope->view()->content_rect() );
    // }

    for (Node* input_node : nodeview->node->inputs() )
    {
        if( !input_node->view )
            continue;
        if( HAS_FLAGS(input_node->view->flags, View_Flag_HIDDEN) )
            continue;
        if( HAS_FLAGS(input_node->view->flags, View_Flag_SELECTED) && HAS_FLAGS(flags, Node_View_Flag_EXCLUDE_UNSELECTED) )
            continue;
        if( HAS_FLAGS(input_node->view->flags, View_Flag_PINNED) && !HAS_FLAGS(flags, Node_View_Flag_WITH_PINNED ) )
            continue;
        if( node_is_output_node_in_expression( input_node, nodeview->node ) )
        {
            result = result.bounding_rect(result,  nodeview_get_rect_ex( input_node->view, space, flags) );
        }
    }

#if NDBL_ASTNODEVIEW_DEBUG_DRAW
    Rect screen_rect = result;
    screen_rect.translate(get_pos(space) - get_pos(PARENT_SPACE) );
    ImGuiEx::DebugRect(screen_rect.min, screen_rect.max, IM_COL32( 0, 255, 0, 60 ), 2 );
#endif

    return result;
}

void ndbl::nodeview_toggle_expandcollapse(Node_View* nodeview)
{
    nodeview->is_expanded = !nodeview->is_expanded;
    nodeview_set_visible_recursively(nodeview, nodeview->is_expanded);
    UNSET_FLAGS(nodeview->flags, tools::View_Flag_HIDDEN);
}

void ndbl::nodeview_set_visible_recursively(Node_View* nodeview, bool visible)
{
    SET_FLAGS_VALUE(nodeview->flags, View_Flag_HIDDEN, !visible );

    // Propagate on inputs unless we reach a different scope
    for( Node_View* input_nodeview : nodeview_get_adjacent(nodeview, Node_Slot::Flag_INPUT ) )
    {
        if( nodeview->node->internal_scope)
        {
            if (nodeview->node->internal_scope == input_nodeview->node->scope || nodeview->node->scope == input_nodeview->node->scope)
                nodeview_set_visible_recursively(input_nodeview, visible);
        }
        else if (nodeview->node->scope == input_nodeview->node->scope && !node_is_connected_to_codeflow(input_nodeview->node) )
        {
            nodeview_set_visible_recursively(input_nodeview, visible);
        }
    }
    
    if ( nodeview->node->internal_scope == nullptr )    
        return;

    // Propagate on scope children
    for (Node* backbone_node: scope_get_backbone(nodeview->node->internal_scope ))
        if ( backbone_node->view )
            nodeview_set_visible_recursively(backbone_node->view, visible );

    // Propagate on branches
    std::set<Scope*> scopes;
    scope_get_descendent(scopes, nodeview->node->internal_scope, 1 );
    for(Node* outputflow_node :  nodeview->node->flow_outputs() )
        if ( outputflow_node->view )
                nodeview_set_visible_recursively( outputflow_node->view, visible );
}

Node_View* ndbl::nodeview_substitute_with_parent_if_not_visible(Node_View* _view, bool _recursive)
{
    if( _view == nullptr )
    {
        return _view;
    }

    if( !HAS_FLAGS(_view->flags, View_Flag_HIDDEN) )
    {
        return _view;
    }

    if ( _recursive )
        if( Scope* scope = _view->node->scope )
            if (Node_View* parent_view = scope->node->view )
                return HAS_FLAGS(parent_view->flags, View_Flag_HIDDEN)
                    ? nodeview_substitute_with_parent_if_not_visible(parent_view, _recursive)
                    : parent_view;

    return nullptr;
}

std::vector<Node_View*> ndbl::nodeview_get_adjacent(const Node_View* nodeview, Node_Slot::Flags flags)
{
    std::vector<Node_View*> result;
        for(auto adjacent_node : node_get_adjacent_nodes( nodeview->node, flags ) )
            if( adjacent_node->view )
                result.push_back( adjacent_node->view );
    return result;
}

void ndbl::nodeview_draw_slot(Node_View* nodeview, Node_Slot_View* slot_view)
{
    nodeslotview_draw(slot_view);

    if( HAS_FLAGS(slot_view->flags, View_Flag_HOVERED) )
    {
        nodeview->hovered_slotview = slot_view; // last wins
    }
}

Node_Property_View* ndbl::nodeview_find_property_view(Node_View* nodeview, const Node_Property* property)
{
    auto found = nodeview->view_by_property.find(property );
    if (found != nodeview->view_by_property.end() )
        return found->second;
    return nullptr;
}

void ndbl::nodeview_reset_all_properties(Node_View* nodeview)
{
    for( auto& [_, property_view] : nodeview->view_by_property )
    {
        nodepropertyview_reset(property_view);
    }
}

