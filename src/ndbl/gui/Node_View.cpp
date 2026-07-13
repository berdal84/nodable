#include "Node_View.h"

#include <algorithm> // for std::max
#include <glm/trigonometric.hpp> // for sinus
#include <vector>

#include "gui/ImGuiEx.h"
#include "gui/View_State.h"
#include "gui/geometry/Space.h"
#include "tools/core/Math.h"

#include "ndbl/core/Node.h"
#include "ndbl/core/Scope.h"

#include "Config.h"
#include "Node_Property_View.h"
#include "Node_Slot_View.h"
#include "Graph_View.h"
#include "Physics_Component.h"


#ifdef NDBL_DEBUG
#define NDBL_ASTNODEVIEW_DEBUG_DRAW 0
#endif

using namespace ndbl;
using namespace tools;

#define PIXEL_PERFECT true // round positions for drawing only

Node_View::Node_View()
: Component<Node>("View")
{
    Component::signal_init.connect<&nodeview_handle_init>(this);
    Component::signal_shutdown.connect<&nodeview_handle_shutdown>(this);
}

Node_View::~Node_View()
{
    Component::signal_init.disconnect();
    Component::signal_shutdown.disconnect();
    assert(slot_views.empty());
    assert(view_by_property.empty());
    for(auto vector : view_by_property_type )
        assert(vector.empty());
}

void ndbl::nodeview_handle_init(Node_View* node_view)
{
    Config* cfg = get_config();

    // 1. Create Property views
    //-------------------------

    VERIFY(node_view->view_by_property.empty(), "Cannot be called twice");

    for (Node_Property* property : node_view->entity->props )
    {
        // Create view
        auto new_view = new Node_Property_View(property);
        node_view->shape.spatial_node()->add_child( new_view->spatial_node() );
        new_view->spatial_node()->set_position({}, tools::PARENT_SPACE);

        switch ( node_view->entity->type )
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
                if ( property->has_flags(Node_Property::Flag_IS_NODE_VALUE) )
                {
                    new_view->state.set_flags(View_Flag_VISIBLE, false);
                }
            }
        }

        // Indexing
        if (property == node_view->entity->value )
        {
            node_view->value_view = new_view;
        }

        bool has_in  = node_find_slot_by_property(node_view->node(), property, Node_Slot::Flag_INPUT );
        bool has_out = node_find_slot_by_property(node_view->node(), property, Node_Slot::Flag_OUTPUT );

        if ( has_in)
            node_view->view_by_property_type[Property_Category_IN].push_back(new_view);
        if ( has_out)
            node_view->view_by_property_type[Property_Category_OUT].push_back(new_view);

        if ( has_in && has_out )
            node_view->view_by_property_type[Property_Category_INOUT_STRICTLY].push_back(new_view);
        else if ( has_in )
            node_view->view_by_property_type[Property_Category_IN_STRICTLY].push_back(new_view);
        else if ( has_out )
            node_view->view_by_property_type[Property_Category_OUT_STRICTLY].push_back(new_view);

        node_view->view_by_property.emplace(property, new_view);
    }

    // 2. Create a Node_Slot_View per slot
    //------------------------------

    for(Node_Slot_View* each : node_view->slot_views )
        delete each;
    node_view->slot_views.clear();

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
    for( Node_Slot* slot : node_view->node()->slots )
    {
        const u8_t index = count_per_type.at(slot->type_and_order())++;
        auto* view = new Node_Slot_View(slot, get_pivot(slot), get_shapetype(slot), index, &node_view->shape );

        node_view->shape.spatial_node()->add_child( view->spatial_node() );
        view->spatial_node()->set_position({}, tools::PARENT_SPACE);
        
        node_view->slot_views.push_back(view);
    }

    // Make sure inputs/outputs are aligned with the property views (if present) and not the node's view.
    for(Node_Slot_View* slot_view : node_view->slot_views)
    {
        switch ( slot_view->slot->type() )
        {
            case Node_Slot::Flag_TYPE_VALUE:
            {
                const Node_Property_View* property_view = nodeview_find_property_view(node_view, slot_view->property());
                if ( property_view != nullptr && property_view->state.has_flags(View_Flag_VISIBLE) )
                    slot_view->alignment_ref = &property_view->shape;
            }
        }
    }

    // Adjust some slot views
    switch ( node_view->node()->type )
    {
        case Node_Type_VARIABLE:
        {
            if ( Node_Slot* decl_out =  node_view->node()->variable_data.decl_out )
            {
                if (Node_Slot_View *view = decl_out->view)
                {
                    view->alignment = LEFT;
                    nodeslotview_update_direction_from_alignment(view);
                    view->alignment_ref = &node_view->shape;
                }
            }
            break;
        }
        case Node_Type_FUNCTION:
        {
            if ( Node_Slot* value_out = node_view->node()->value_out() )
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
    node_view->colors[Color_FILL] = &cfg->ui_node_fill_color[ node_view->node()->type];

    // 4. Create Scope_View
    //--------------------

    if ( Scope* internal_scope = node_view->node()->internal_scope )
    {
        auto* scopeview = new Scope_View();
        scopeview_init(scopeview, internal_scope);

        node_view->shape.spatial_node()->add_child( &scopeview->spatial_node );
        scopeview->spatial_node.set_position({0.f, 0.f}, tools::PARENT_SPACE);

        node_view->internal_scopeview = scopeview;
    }
}

void ndbl::nodeview_handle_shutdown(Node_View* node_view)
{
    node_view->spatial_node()->clear();

    for(auto& [_, each] : node_view->view_by_property )
        delete each;
    node_view->view_by_property.clear();

    for(auto& vector : node_view->view_by_property_type )
        vector.clear();
    // no m_view_by_property_type.clear(), it is an array ;)

    for(auto* each : node_view->slot_views )
        delete each;
    node_view->slot_views.clear();

    if( node_view->internal_scopeview != nullptr )
        scopeview_shutdown(node_view->internal_scopeview);

    node_view->hovered_slotview = nullptr;
}

std::string ndbl::nodeview_get_label(const Node_View* node_view)
{
    Config* cfg = get_config();

    bool minimalist = cfg->ui_node_detail == View_Detail::MINIMALIST;

    switch (node_view->node()->type )
    {
        case Node_Type_VARIABLE_REF:
        {
            if ( minimalist )
                return "&";
            return node_view->node()->name;
        }
        case Node_Type_VARIABLE:
        {
            if (minimalist)
                return "";
            return node_variable_type(node_view->node() )->name();
        }
        case Node_Type_OPERATOR:
        {
            return node_view->node()->name;
        }
        case Node_Type_FUNCTION:
        {
            if ( minimalist )
                return "f(x)";
            return node_view->node()->name;
        }
        case Node_Type_ROOT:
        case Node_Type_SCOPE:
        {
            if ( minimalist )
            {
                return node_view->node()->name.substr(0, 6); // 4 char for the icon
            }
            return node_view->node()->name;
        }
        case Node_Type_IF_ELSE:
        {
            if ( minimalist )
                return "?";
            return node_view->node()->name;
        }
        case Node_Type_FOR_LOOP:
        {
            if ( minimalist )
                return "for";
            return node_view->node()->name;
        }
        default:
        {
            if ( minimalist )
                return node_view->node()->name.substr(0, 3) + ".";
            return node_view->node()->name;
        }
    }

}

void ndbl::nodeview_arrange_recursively(Node_View* node_view, bool _smoothly)
{
    for (auto each_input: nodeview_get_adjacent(node_view, Node_Slot::Flag_INPUT) )
    {
        if ( !each_input->state.has_flags(View_Flag_PINNED) )
            if (node_is_output_node_in_expression(each_input->node(), node_view->node() ) )
                nodeview_arrange_recursively(each_input);
    }

    if (Scope* internal_scope = node_view->node()->internal_scope )
        for ( Node* _node : scope_get_backbone(internal_scope) )
            if ( auto* _node_view = componentbag_get<Node_View>(&_node->component_bag))
                    nodeview_arrange_recursively(_node_view);

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well-placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        nodeview_update(node_view, float(1000));
    }

    node_view->state.set_flags(View_Flag_PINNED, false);
}

void ndbl::nodeview_update(Node_View* node_view, float dt)
{
    if( node_view->opacity != 1.0f)
        tools::clamped_lerp(node_view->opacity, 1.0f, 10.0f * dt);

    for(Node_Slot_View* each_slot_view  : node_view->slot_views )
        nodeslotview_update( each_slot_view, dt );

    if ( node_view->internal_scopeview != nullptr )
        scopeview_update( node_view->internal_scopeview, dt );
}

bool ndbl::nodeview_draw(Node_View* node_view)
{
    node_view->shape.draw_debug_info();

    if ( !node_view->state.has_flags(View_Flag_VISIBLE) )
        return false;

    ASSERT( node_view->node() );

    Config*     cfg       = get_config();
	bool        changed   = false;

    node_view->hovered_slotview = nullptr; // reset every frame

    // Draw background slots (rectangles)
    for( Node_Slot_View* slot_view : node_view->slot_views )
        if ( slot_view->shape_type == Shape_Type_RECTANGLE)
            nodeview_draw_slot(node_view, slot_view);

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, node_view->opacity);
    Rect screen_rect = node_view->shape.rect(WORLD_SPACE);

#if PIXEL_PERFECT
        screen_rect.min.round();
        screen_rect.max.round();
#endif

    ImGui::SetCursorScreenPos( screen_rect.top_left() ); // start from th top left corner
	ImGui::PushID(node_view);


	// Draw the background of the Group
    Vec4 border_color = cfg->ui_node_borderColor;
    if ( node_view->state.has_flags(View_Flag_SELECTED) )
    {
        border_color = cfg->ui_node_borderHighlightedColor;
    }
    else if ( node_is_instruction( node_view->node() ) )
    {
        border_color = cfg->ui_node_fill_color[Node_Type_NULL];
    }

    float border_width = cfg->ui_node_borderWidth;
    if( node_is_instruction( node_view->node() ) )
    {
        border_width *= cfg->ui_node_instructionBorderRatio;
    }

    nodeview_draw_node_rect(
            screen_rect,
            *node_view->colors[Color_FILL],
            cfg->ui_node_borderColor,
            cfg->ui_node_shadowColor,
            border_color,
            node_view->state.has_flags(View_Flag_SELECTED),
            5.0f,
            border_width );

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

    std::string pre_label;
    std::vector<std::string> operator_label(1); // for binary (and ternary when implemented) operators
    std::string post_label;

    switch ( node_view->node()->type )
    {
        case Node_Type_OPERATOR:
            if ( node_is_unary_operator(node_view->node() ) )
                pre_label = nodeview_get_label(node_view);
            else if ( node_is_binary_operator(node_view->node() ) )
                operator_label[0] = nodeview_get_label(node_view);
            // else if (node->is_ternary_operator()
            break;
        default:
            pre_label = nodeview_get_label(node_view);
            break;
        case Node_Type_FUNCTION:
            pre_label = nodeview_get_label(node_view);
            post_label = "";

            if ( cfg->ui_node_detail != View_Detail::MINIMALIST)
            {
                pre_label.push_back('(');
                post_label.push_back(')');
            }
            break;
    }

    if ( !node_view->is_expanded )
        pre_label.append(" " ICON_FA_OBJECT_GROUP);

    // Draw the pre_label when necessary
    if ( !pre_label.empty() ) {
        ImGui::SameLine();
        ImGui::Text("%s", pre_label.c_str());

        // Update slot_view_out to be positioned below the pre_label

        if (node_view->node()->type == Node_Type_FUNCTION )
            if (Node_Slot *slot_out = node_view->node()->value_out())
                if (Node_Slot_View *slot_view_out = slot_out->view)
                {
                    const float x = ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x * 0.5f;
                    const float y = node_view->shape.pivot(BOTTOM, WORLD_SPACE).y;
                    slot_view_out->spatial_node()->set_position({x, y}, WORLD_SPACE);
                    slot_view_out->direction = BOTTOM;
                }
    }

    // Draw the properties depending on node type
    if (node_view->node()->type != Node_Type_OPERATOR )
    {
        changed |= nodepropertyview_draw_all(node_view->view_by_property_type[Property_Category_IN_STRICTLY]    , cfg->ui_node_detail);
        changed |= nodepropertyview_draw_all(node_view->view_by_property_type[Property_Category_INOUT_STRICTLY] , cfg->ui_node_detail);
        changed |= nodepropertyview_draw_all(node_view->view_by_property_type[Property_Category_OUT_STRICTLY]   , cfg->ui_node_detail);
    }
    else
    {
        size_t i = 0;
        for( Node_Property_View* property_view : node_view->view_by_property_type[Property_Category_IN] )
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

    node_view->shape.set_size(Vec2::round(new_size));

    // Draw foreground slots (circles)
    for( Node_Slot_View* slot_view: node_view->slot_views )
        if ( slot_view->shape_type == Shape_Type_CIRCLE)
            nodeview_draw_slot(node_view, slot_view);

	ImGui::PopStyleVar();
	ImGui::PopID();

    if ( changed )
        node_view->node()->set_flags(Node_Flag_IS_DIRTY );

    const bool _hovered = is_rect_hovered || node_view->hovered_slotview != nullptr;
    node_view->state.set_flags(View_Flag_HOVERED, _hovered );

	return changed;
}

void ndbl::nodeview_draw_node_rect(
    Rect rect,
    Vec4 color,
    Vec4 border_highlight_col,
    Vec4 shadow_col,
    Vec4 border_col,
    bool selected,
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

    // Draw an additional blinking rectangle when selected
    if (selected)
    {
        auto alpha   = glm::sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
        float offset = 4.0f;
        draw_list->AddRect(
            rect.min - Vec2(offset),
            rect.max + Vec2(offset),
            ImColor(1.0f, 1.0f, 1.0f, float(alpha) ),
            border_radius + offset,
            ~0,
            offset / 2.0f
        );
    }
}

bool ndbl::nodeview_draw_as_properties_panel(Node_View* node_view, bool* _show_advanced)
{
    bool changed = false;
    Node* node = node_view->node();
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](Node_Property_View* _property_view) -> bool
    {
        Node_Property* property = _property_view->property;
        // label (<name> (<type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s): ",
                property->name.c_str(),
                property->type->name());

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

    ImGui::Text("Name:       \"%s\"" , node->name.c_str());
    ImGui::Text("Class:      %s"     , node->get_class()->name());

    // Draw exposed input properties

    auto draw_properties = [&](const char* title, const std::vector<Node_Property_View*>& views) -> bool
    {
        bool changed = false;
        ImGui::Text("%s:", title);
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
    changed |= draw_properties("Inputs(s)", node_view->view_by_property_type[Property_Category_IN_STRICTLY]);
    changed |= draw_properties("In/Out(s)", node_view->view_by_property_type[Property_Category_INOUT_STRICTLY]);
    ImGui::Separator();
    changed |= draw_properties("Output(s)", node_view->view_by_property_type[Property_Category_OUT_STRICTLY]);

#ifdef NDBL_DEBUG

    ImGui::Separator();
    changed |= draw_labeled_property_view( node_view->value_view );
    ImGui::Separator();

    ImGui::Separator();
    ImGui::Text("Component(s) (%zu)", node->component_bag.size() );
    ImGui::Separator();
    for (Component<Node>* component : node->component_bag )
    {
        if( ImGui::TreeNode(component, "Component %s", component->name ) )
        {
            if ( component != *node->component_bag.begin() )
                ImGui::Separator();

            if ( component->type_desc == type::get<Physics_Component>())
            {
                auto* physics_component = reinterpret_cast<Physics_Component*>( component );

                ImGui::Checkbox("On/Off", &physics_component->is_active());

                // for (ViewConstraint& constraint: physics_component->constraints())
                // {
                //     if (ImGui::TreeNode(&constraint, "%s", constraint.name) )
                //     {
                //         ImGui::Checkbox("enabled", &constraint.enabled);
                //         ImGui::TreePop();
                //     }
                // }
            }
            else if (component->type_desc == type::get<Scope>())
            {
                TreeNode_Scope("Scope", static_cast<Scope *>( component ));
            }
            ImGui::TreePop();
        }
    }
    ImGui::Separator();

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
                String_128 label;
                label.append_fmt("%s %p (%s %p)", scope->name, scope, scope->entity->name.c_str(), scope->entity);
                if ( ImGui::Button(label.c_str()) )
                {
                    Graph_View* graph_view = componentbag_get<Graph_View>(&node->graph->component_bag);
                    ASSERT(graph_view);
                    graph_view->selection.clear();
                    graph_view->selection.append(componentbag_get<Node_View>(&scope->entity->component_bag) );
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
            ImGui::Text(node->has_flags(Node_Flag_IS_DIRTY) ? "yes" : "no");

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

Rect ndbl::nodeview_get_rect_ex(const Node_View* node_view, tools::Space space, Node_View_Flags flags)
{
    if( (flags & Node_View_Flag_WITH_RECURSION) == 0 )
        return node_view->shape.rect(space);

    Rect result;

    if ( node_view->state.has_flags(View_Flag_VISIBLE) )
    {
        result = node_view->shape.rect(space);
    }

    // TEMPORARILY DISABLED: we need a flag here to disable this type of "INNER_RECURSION"
    // if ( Scope* _internal_scope = node()->internal_scope() )
    // {
    //      result = result.bounding_rect(result, _internal_scope->view()->content_rect() );
    // }

    for (Node* input_node : node_view->node()->inputs() )
    {
        auto* view = componentbag_get<Node_View>(&input_node->component_bag);
        if( !view )
            continue;
        if( !view->state.has_flags(View_Flag_VISIBLE) )
            continue;
        if( view->state.has_flags(View_Flag_SELECTED) && (flags & Node_View_Flag_EXCLUDE_UNSELECTED) )
            continue;
        if( view->state.has_flags(View_Flag_PINNED) && (flags & Node_View_Flag_WITH_PINNED ) == 0 )
            continue;
        if( node_is_output_node_in_expression( input_node, node_view->node() ) )
        {
            result = result.bounding_rect(result,  nodeview_get_rect_ex( view, space, flags) );
        }
    }

#if NDBL_ASTNODEVIEW_DEBUG_DRAW
    Rect screen_rect = result;
    screen_rect.translate(get_pos(space) - get_pos(PARENT_SPACE) );
    ImGuiEx::DebugRect(screen_rect.min, screen_rect.max, IM_COL32( 0, 255, 0, 60 ), 2 );
#endif

    return result;
}

Rect ndbl::nodeview_bounding_rect(
    const std::vector<Node_View *>& view,
    Space space,
    Node_View_Flags flags
)
{
    // collect rectangles
    // note: we could save 1 allocation by computing the bbox of each rectangle instead of building this vector,
    //       but I prefer to keep responsibilities separated.
    std::vector<Rect> rect;
    rect.reserve(view.size());
    for (size_t i = 0; i < view.size(); ++i)
    {
        rect.emplace_back( nodeview_get_rect_ex(view[i], space, flags) ) ;
    }
    // compute bbox
    return Rect::bounding_rect(rect);
}

void ndbl::nodeview_set_expanded_rec(Node_View* node_view, bool _expanded)
{
    nodeview_set_expanded(node_view, _expanded);

    if ( Scope* _internal_scope = node_view->node()->internal_scope )
        for( Node* _node : scope_get_backbone(_internal_scope) )
            if ( auto* view = componentbag_get<Node_View>(&_node->component_bag) )
                nodeview_set_expanded_rec(view, _expanded);
}

void ndbl::nodeview_set_expanded(Node_View* node_view, bool expand)
{
    node_view->is_expanded = expand;
    nodeview_set_inputs_visible(node_view, expand, true);
    nodeview_set_children_visible(node_view, expand, true);
}

void ndbl::nodeview_set_inputs_visible(Node_View* node_view, bool _visible, bool _recursive)
{
    nodeview_set_adjacent_visible(node_view, Node_Slot::Flag_INPUT, _visible, Node_View_Flag_WITH_RECURSION * _recursive);
}

void ndbl::nodeview_set_children_visible(Node_View* node_view, bool visible, bool recursively)
{
    if ( node_view->node()->internal_scope == nullptr )
        return;

    std::set<Scope*> scopes;
    scope_get_descendent(scopes, node_view->node()->internal_scope, 1 );

    for(Scope* each_scope : scopes)
        for (Node* each_child_node: scope_get_backbone(each_scope))
            if ( auto* view = componentbag_get<Node_View>(&each_child_node->component_bag) )
                view->state.set_flags(View_Flag_VISIBLE, visible );
}

void ndbl::nodeview_set_adjacent_visible(Node_View* node_view, Node_Slot::Flags slot_flags, bool _visible, Node_View_Flags node_flags)
{
    bool has_not_output = node_view->node()->outputs().empty();
    for( Node_View* each_child_view : nodeview_get_adjacent(node_view, slot_flags) )
    {
        if(_visible || has_not_output || node_is_output_node_in_expression(each_child_view->node(),
                                                                                node_view->node()) )
        {
            if ( (node_flags & Node_View_Flag_WITH_RECURSION) && each_child_view->is_expanded ) // propagate only if expanded
            {
                nodeview_set_children_visible(each_child_view,_visible, true);
                nodeview_set_inputs_visible(each_child_view, _visible, true);
            }
            each_child_view->state.set_flags(View_Flag_VISIBLE,_visible );
        }
    }
}

Node_View* ndbl::nodeview_substitute_with_parent_if_not_visible(Node_View* _view, bool _recursive)
{
    if( _view == nullptr )
    {
        return _view;
    }

    if( _view->state.has_flags(View_Flag_VISIBLE) )
    {
        return _view;
    }

    if ( _recursive )
        if( Scope* scope = _view->node()->scope )
            if (Node_View* parent_view = componentbag_get<Node_View>(&scope->entity->component_bag) )
                return parent_view->state.has_flags(View_Flag_VISIBLE) ? parent_view
                                                      : nodeview_substitute_with_parent_if_not_visible(parent_view, _recursive);

    return nullptr;
}

std::vector<Node_View*> ndbl::nodeview_get_adjacent(const Node_View* node_view, Node_Slot::Flags flags)
{
    std::vector<Node_View*> result;
        for(auto _adjacent_node : node_get_adjacent_nodes( node_view->node(), flags ) )
            if( auto* component = componentbag_get<Node_View>(&_adjacent_node->component_bag) )
                result.push_back( component );
    return result;
}

void ndbl::nodeview_draw_slot(Node_View* node_view, Node_Slot_View* slot_view)
{
    nodeslotview_draw(slot_view);

    if( slot_view->state.has_flags(View_Flag_HOVERED) )
    {
        node_view->hovered_slotview = slot_view; // last wins
    }
}

Node_Property_View* ndbl::nodeview_find_property_view(Node_View* node_view, const Node_Property* property)
{
    auto found = node_view->view_by_property.find(property );
    if (found != node_view->view_by_property.end() )
        return found->second;
    return nullptr;
}

void ndbl::nodeview_reset_all_properties(Node_View* node_view)
{
    for( auto& [_, property_view] : node_view->view_by_property )
    {
        nodepropertyview_reset(property_view);
    }
}

