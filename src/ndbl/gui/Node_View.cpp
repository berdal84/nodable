#include "Node_View.h"

#include <algorithm> // for std::max
#include <glm/trigonometric.hpp> // for sinus
#include <vector>

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
    Component::signal_init.connect<&Node_View::_handle_init>(this);
    Component::signal_shutdown.connect<&Node_View::_handle_shutdown>(this);
}

Node_View::~Node_View()
{
    Component::signal_init.disconnect();
    Component::signal_shutdown.disconnect();
    assert(m_slot_views.empty());
    assert(m_view_by_property.empty());
    for(auto vector : m_view_by_property_type )
        assert(vector.empty());
}

void Node_View::_handle_init()
{
    Config* cfg = get_config();

    // 1. Create Property views
    //-------------------------

    VERIFY(m_view_by_property.empty(), "Cannot be called twice");

    for (Node_Property* property : node()->props )
    {
        // Create view
        auto new_view = new Node_Property_View(property);
        _add_child(new_view);

        switch ( node()->type )
        {
            case Node_Type_ROOT:
            case Node_Type_SCOPE:
            case Node_Type_FUNCTION:
            case Node_Type_OPERATOR:
            case Node_Type_FOR_LOOP:
            case Node_Type_IF_ELSE:
            case Node_Type_WHILE_LOOP:
                // hide THIS property
                if ( property->has_flags(Node_Property::Flag_IS_NODE_VALUE) )
                    new_view->state()->set_visible(false);
        }

        // Indexing
        if (property == node()->value )
        {
            m_value_view = new_view;
        }

        bool has_in  = node_find_slot_by_property(node(), property, Node_Slot::Flag_INPUT );
        bool has_out = node_find_slot_by_property(node(), property, Node_Slot::Flag_OUTPUT );

        if ( has_in)
            m_view_by_property_type[PropType_IN].push_back(new_view);
        if ( has_out)
            m_view_by_property_type[PropType_OUT].push_back(new_view);

        if ( has_in && has_out )
            m_view_by_property_type[PropType_INOUT_STRICTLY].push_back(new_view);
        else if ( has_in )
            m_view_by_property_type[PropType_IN_STRICTLY].push_back(new_view);
        else if ( has_out )
            m_view_by_property_type[PropType_OUT_STRICTLY].push_back(new_view);

        m_view_by_property.emplace(property, new_view);
    }

    // 2. Create a Node_Slot_View per slot
    //------------------------------

    for(auto* each : m_slot_views )
        delete each;
    m_slot_views.clear();

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
    for( Node_Slot* slot : node()->slots )
    {
        const u8_t index = count_per_type.at(slot->type_and_order())++;
        auto* view = new Node_Slot_View(slot, get_pivot(slot), get_shapetype(slot), index, shape() );
        _add_child(view);
    }

    // Make sure inputs/outputs are aligned with the property views (if present) and not the node's view.
    for(auto view : m_slot_views)
    {
        switch ( view->slot->type() )
        {
            case Node_Slot::Flag_TYPE_VALUE:
            {
                const Node_Property_View* property_view = _find_property_view(view->property());
                if ( property_view != nullptr && property_view->state()->visible() )
                    view->alignment_ref = property_view->shape();
            }
        }
    }

    // Adjust some slot views
    switch ( node()->type )
    {
        case Node_Type_VARIABLE:
        {
            if ( Node_Slot* decl_out = node()->variable_data().decl_out )
            {
                if (Node_Slot_View *view = decl_out->view)
                {
                    view->alignment = LEFT;
                    view->update_direction_from_alignment();
                    view->alignment_ref = this->shape();
                }
            }
            break;
        }
        case Node_Type_FUNCTION:
        {
            if ( Node_Slot* value_out = node()->value_out() )
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
    set_color( &cfg->ui_node_fill_color[ node()->type] );

    // 4. Create Scope_View
    //--------------------

    if ( Scope* internal_scope = node()->internal_scope )
    {
        auto* scopeview = new Scope_View();
        scopeview->init(internal_scope);
        _add_child(scopeview);
        m_internal_scopeview = scopeview;
    }
}

void Node_View::_handle_shutdown()
{
    spatial_node()->clear();

    for(auto& [_, each] : m_view_by_property )
        delete each;
    m_view_by_property.clear();

    for(auto& vector : m_view_by_property_type )
        vector.clear();
    // no m_view_by_property_type.clear(), it is an array ;)

    for(auto* each : m_slot_views )
        delete each;
    m_slot_views.clear();

    if(m_internal_scopeview )
        m_internal_scopeview->shutdown();

    m_hovered_slotview = nullptr;
}

std::string Node_View::get_label()
{
    Config* cfg = get_config();

    bool minimalist = cfg->ui_node_detail == View_Detail::MINIMALIST;

    switch (node()->type )
    {
        case Node_Type_VARIABLE_REF:
        {
            if ( minimalist )
                return "&";
            return node()->name;
        }
        case Node_Type_VARIABLE:
        {
            if (minimalist)
                return "";
            return node_variable_type(node() )->name();
        }
        case Node_Type_OPERATOR:
        {
            return node()->name;
        }
        case Node_Type_FUNCTION:
        {
            if ( minimalist )
                return "f(x)";
            return node()->name;
        }
        case Node_Type_ROOT:
        case Node_Type_SCOPE:
        {
            if ( minimalist )
            {
                return node()->name.substr(0, 6); // 4 char for the icon
            }
            return node()->name;
        }
        case Node_Type_IF_ELSE:
        {
            if ( minimalist )
                return "?";
            return node()->name;
        }
        case Node_Type_FOR_LOOP:
        {
            if ( minimalist )
                return "for";
            return node()->name;
        }
        default:
        {
            if ( minimalist )
                return node()->name.substr(0, 3) + ".";
            return node()->name;
        }
    }

}

void Node_View::arrange_recursively(bool _smoothly)
{
    for (auto each_input: get_adjacent(Node_Slot::Flag_INPUT) )
    {
        if ( !each_input->m_view_state.pinned() )
            if (node_is_output_node_in_expression(each_input->node(), node() ) )
                each_input->arrange_recursively();
    }

    if (Scope* internal_scope = node()->internal_scope )
        for ( Node* _node : internal_scope->backbone() )
            if ( auto* _node_view = _node->component<Node_View>() )
                    _node_view->arrange_recursively();

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well-placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        update(float(1000));
    }

    m_view_state.set_pinned(false);
}

void Node_View::update(float dt)
{
    if(m_opacity != 1.0f)
        tools::clamped_lerp(m_opacity, 1.0f, 10.0f * dt);

    for(Node_Slot_View* _slotview  : m_slot_views )
        _slotview->update( dt );

    if ( m_internal_scopeview )
        m_internal_scopeview->update( dt );
}

bool Node_View::draw()
{
    m_shape.draw_debug_info();

    if ( !m_view_state.visible() )
        return false;

    ASSERT( node() );

    Config*     cfg       = get_config();
	bool        changed   = false;

    m_hovered_slotview    = nullptr; // reset every frame

    // Draw background slots (rectangles)
    for( Node_Slot_View* slot_view: m_slot_views )
        if ( slot_view->shape_type == Shape_Type_RECTANGLE)
            _draw_slot(slot_view);

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
    Rect screen_rect = get_rect(WORLD_SPACE);

#if PIXEL_PERFECT
        screen_rect.min.round();
        screen_rect.max.round();
#endif

    ImGui::SetCursorScreenPos( screen_rect.top_left() ); // start from th top left corner
	ImGui::PushID(this);


	// Draw the background of the Group
    Vec4 border_color = cfg->ui_node_borderColor;
    if ( m_view_state.selected() )
    {
        border_color = cfg->ui_node_borderHighlightedColor;
    }
    else if ( node_is_instruction(node() ) )
    {
        border_color = cfg->ui_node_fill_color[Node_Type_NULL];
    }

    float border_width = cfg->ui_node_borderWidth;
    if( node_is_instruction(node() ) )
    {
        border_width *= cfg->ui_node_instructionBorderRatio;
    }

    DrawNodeRect(
            screen_rect,
            get_color( Color_FILL ),
            cfg->ui_node_borderColor,
            cfg->ui_node_shadowColor,
            border_color,
            m_view_state.selected(),
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

    switch ( node()->type )
    {
        case Node_Type_OPERATOR:
            if ( node_is_unary_operator(node() ) )
                pre_label = get_label();
            else if ( node_is_binary_operator(node() ) )
                operator_label[0] = get_label();
            // else if (node->is_ternary_operator()
            break;
        default:
            pre_label = get_label();
            break;
        case Node_Type_FUNCTION:
            pre_label = get_label();
            post_label = "";

            if ( cfg->ui_node_detail != View_Detail::MINIMALIST)
            {
                pre_label.push_back('(');
                post_label.push_back(')');
            }
            break;
    }

    if ( !m_expanded )
        pre_label.append(" " ICON_FA_OBJECT_GROUP);

    // Draw the pre_label when necessary
    if ( !pre_label.empty() ) {
        ImGui::SameLine();
        ImGui::Text("%s", pre_label.c_str());

        // Update slot_view_out to be positioned below the pre_label

        if (node()->type == Node_Type_FUNCTION )
            if (Node_Slot *slot_out = node()->value_out())
                if (Node_Slot_View *slot_view_out = slot_out->view)
                {
                    const float x = ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x * 0.5f;
                    const float y = shape()->pivot(BOTTOM, WORLD_SPACE).y;
                    slot_view_out->spatial_node()->set_position({x, y}, WORLD_SPACE);
                    slot_view_out->direction = BOTTOM;
                }
    }

    // Draw the properties depending on node type
    if (node()->type != Node_Type_OPERATOR )
    {
        changed |= Node_Property_View::draw_all(m_view_by_property_type[PropType_IN_STRICTLY], cfg->ui_node_detail);
        changed |= Node_Property_View::draw_all(m_view_by_property_type[PropType_INOUT_STRICTLY], cfg->ui_node_detail);
        changed |= Node_Property_View::draw_all(m_view_by_property_type[PropType_OUT_STRICTLY], cfg->ui_node_detail);
    }
    else
    {
        size_t i = 0;
        for( Node_Property_View* property_view : m_view_by_property_type[PropType_IN] )
        {
            ImGui::SameLine();
            changed |= property_view->draw( cfg->ui_node_detail );

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

    shape()->set_size(Vec2::round(new_size));

    // Draw foreground slots (circles)
    for( Node_Slot_View* slot_view: m_slot_views )
        if ( slot_view->shape_type == Shape_Type_CIRCLE)
            _draw_slot(slot_view);

	ImGui::PopStyleVar();
	ImGui::PopID();

    if ( changed )
        node()->set_flags(Node_Flag_IS_DIRTY );

    const bool _hovered = is_rect_hovered || m_hovered_slotview != nullptr;
    m_view_state.set_hovered(_hovered );

	return changed;
}

void Node_View::DrawNodeRect(
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

bool Node_View::draw_as_properties_panel(Node_View *_view, bool* _show_advanced)
{
    bool changed = false;
    Node* node = _view->node();
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](Node_Property_View* _property_view) -> bool
    {
        Node_Property*property = _property_view->get_property();
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
        return Node_Property_View::draw_input(_property_view, !_show_advanced, nullptr);
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
    changed |= draw_properties("Inputs(s)", _view->m_view_by_property_type[PropType_IN_STRICTLY]);
    changed |= draw_properties("In/Out(s)", _view->m_view_by_property_type[PropType_INOUT_STRICTLY]);
    ImGui::Separator();
    changed |= draw_properties("Output(s)", _view->m_view_by_property_type[PropType_OUT_STRICTLY]);

#ifdef NDBL_DEBUG

    ImGui::Separator();
    changed |= draw_labeled_property_view( _view->m_value_view );
    ImGui::Separator();

    ImGui::Separator();
    ImGui::Text("Component(s) (%zu)", node->components.size() );
    ImGui::Separator();
    for (Component<Node>* component : node->components )
    {
        if( ImGui::TreeNode(component, "Component %s", component->name().c_str() ) )
        {
            if ( component != *node->components.begin() )
                ImGui::Separator();

            if ( component->get_class() == type::get<Physics_Component>())
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
            else if (component->get_class() == type::get<Scope>())
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
                Node* _node = scope->entity();
                label.append_fmt("%s %p (%s %p)", scope->name().c_str(), scope, _node->name.c_str(), _node);
                if ( ImGui::Button(label.c_str()) )
                {
                    Graph_View* graph_view = node->graph->component<Graph_View>();
                    ASSERT(graph_view);
                    graph_view->selection().clear();
                    graph_view->selection().append(_node->component<Node_View>() );
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

Rect Node_View::get_rect_ex(tools::Space space, Node_ViewFlags flags) const
{
    if( (flags & Node_ViewFlag_WITH_RECURSION) == 0 )
        return this->get_rect(space);

    Rect result;

    if ( m_view_state.visible() )
    {
        result = this->get_rect(space);
    }

    // TEMPORARILY DISABLED: we need a flag here to disable this type of "INNER_RECURSION"
    // if ( Scope* _internal_scope = node()->internal_scope() )
    // {
    //      result = result.bounding_rect(result, _internal_scope->view()->content_rect() );
    // }

    for (Node* input_node : node()->inputs() )
    {
        auto* view = input_node->component<Node_View>();
        if( !view )
            continue;
        if( !view->m_view_state.visible() )
            continue;
        if( view->m_view_state.selected() && (flags & Node_ViewFlag_EXCLUDE_UNSELECTED) )
            continue;
        if( view->m_view_state.pinned() && (flags & Node_ViewFlag_WITH_PINNED ) == 0 )
            continue;
        if( node_is_output_node_in_expression( input_node, node() ) )
        {
            result = result.bounding_rect(result,  view->get_rect_ex(space, flags) );
        }
    }

#if NDBL_ASTNODEVIEW_DEBUG_DRAW
    Rect screen_rect = result;
    screen_rect.translate(get_pos(space) - get_pos(PARENT_SPACE) );
    ImGuiEx::DebugRect(screen_rect.min, screen_rect.max, IM_COL32( 0, 255, 0, 60 ), 2 );
#endif

    return result;
}

Rect
Node_View::bounding_rect(
    const std::vector<Node_View *>& view,
    Space space,
    Node_ViewFlags flags
)
{
    // collect rectangles
    // note: we could save 1 allocation by computing the bbox of each rectangle instead of building this vector,
    //       but I prefer to keep responsibilities separated.
    std::vector<Rect> rect;
    rect.reserve(view.size());
    for (size_t i = 0; i < view.size(); ++i)
    {
        rect.emplace_back( view[i]->get_rect_ex(space, flags) ) ;
    }
    // compute bbox
    return Rect::bounding_rect(rect);
}

void Node_View::set_expanded_rec(bool _expanded)
{
    set_expanded(_expanded);

    if ( Scope* _internal_scope = node()->internal_scope )
        for( Node* _node : _internal_scope->backbone() )
            if ( auto* view = _node->component<Node_View>() )
                view->set_expanded_rec(_expanded);
}

void Node_View::set_expanded(bool _expanded)
{
    m_expanded = _expanded;
    set_inputs_visible(_expanded, true);
    set_children_visible(_expanded, true);
}

void Node_View::set_inputs_visible(bool _visible, bool _recursive)
{
    _set_adjacent_visible(Node_Slot::Flag_INPUT, _visible, Node_ViewFlag_WITH_RECURSION * _recursive);
}

void Node_View::set_children_visible(bool visible, bool recursively)
{
    if ( node()->internal_scope == nullptr )
        return;

    std::set<Scope*> scopes;
    Scope::get_descendent(scopes, node()->internal_scope, 1 );

    for(Scope* _scope : scopes)
        for (Node* _child_node: _scope->backbone())
            if ( auto* view = _child_node->component<Node_View>() )
                view->state()->set_visible(visible );
}

void Node_View::_set_adjacent_visible(Node_Slot::Flags slot_flags, bool _visible, Node_ViewFlags node_flags)
{
    bool has_not_output = node()->outputs().empty();
    for( auto each_child_view : get_adjacent(slot_flags) )
    {
        if(_visible || has_not_output || node_is_output_node_in_expression(each_child_view->node(),
                                                                                this->node()) )
        {
            if ( (node_flags & Node_ViewFlag_WITH_RECURSION) && each_child_view->m_expanded ) // propagate only if expanded
            {
                each_child_view->set_children_visible(_visible, true);
                each_child_view->set_inputs_visible(_visible, true);
            }
            each_child_view->m_view_state.set_visible(_visible );
        }
    }
}

Node_View* Node_View::substitute_with_parent_if_not_visible(Node_View* _view, bool _recursive)
{
    if( _view == nullptr )
    {
        return _view;
    }

    if( _view->m_view_state.visible() )
    {
        return _view;
    }

    if ( _recursive )
        if( Scope* scope = _view->node()->scope )
            if (Node_View* parent_view = scope->entity()->component<Node_View>() )
                return parent_view->m_view_state.visible() ? parent_view
                                                      : substitute_with_parent_if_not_visible(parent_view, _recursive);

    return nullptr;
}

std::vector<Node_View*> Node_View::get_adjacent(Node_Slot::Flags flags) const
{
    std::vector<Node_View*> result;
        for(auto _adjacent_node : node_get_adjacent_nodes( node(), flags ) )
            if( auto* component = _adjacent_node->component<Node_View>() )
                result.push_back( component );
    return result;
}

void Node_View::set_color(const Vec4* _color, ColorType _type )
{
    ASSERT(_color != nullptr);
    m_colors[_type] = _color;
}

Vec4 Node_View::get_color(ColorType _type ) const
{
     auto* color = m_colors[_type];
     VERIFY(color != nullptr, "Did you called set_color(...) ?");
     return *color;
}

void Node_View::_draw_slot(Node_Slot_View* slot_view)
{
    slot_view->draw();

    if( slot_view->state()->hovered() )
    {
        m_hovered_slotview = slot_view; // last wins
    }
}

Node_Property_View *Node_View::_find_property_view(const Node_Property* property)
{
    auto found = m_view_by_property.find(property );
    if (found != m_view_by_property.end() )
        return found->second;
    return nullptr;
}

void Node_View::reset_all_properties()
{
    for( auto& [_, property_view] : m_view_by_property )
        property_view->reset();
}

