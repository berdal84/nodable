#include "NodeView.h"

#include <algorithm> // for std::max
#include <cmath> // for sinus
#include <vector>

#include "tools/core/math.h"
#include "ndbl/core/Utils.h"
#include "ndbl/core/FunctionNode.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/language/Nodlang.h"

#include "Config.h"
#include "Event.h"
#include "Physics.h"
#include "PropertyView.h"
#include "GraphView.h"
#include "SlotView.h"
#include "tools/gui/Config.h"
#include "ndbl/core/Interpreter.h"

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 0
#endif

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(NodeView).extends<NodeComponent>();
)

constexpr Vec4 DEFAULT_COLOR = Vec4(1.f, 0.f, 0.f);
#define PIXEL_PERFECT true // round positions for drawing only

NodeView::NodeView()
    : NodeComponent()
    , m_colors({&DEFAULT_COLOR})
    , m_opacity(1.0f)
    , m_expanded(true)
    , m_pinned(false)
    , m_value_view(nullptr)
    , m_view_by_property()
    , m_hovered_slotview(nullptr)
    , m_last_clicked_slotview(nullptr)
    , m_state(10.0f, 35.0f)
{
    CONNECT( this->on_reset_owner, &NodeView::reset );
}

NodeView::~NodeView()
{
    for(auto& [_, each] : m_view_by_property )
    {
        spatial_node().remove_child(&each->spatial_node());
        delete each;
    }

    for(auto vector : m_view_by_property_type )
    {
        vector.clear();
    }

    for(auto* each : m_slot_views )
    {
        spatial_node().remove_child( &each->spatial_node() );
        delete each;
    }
    m_slot_views.clear();
    m_hovered_slotview      = nullptr;
    m_last_clicked_slotview = nullptr;
}

std::string NodeView::get_label()
{
    Config* cfg = get_config();

    bool minimalist = cfg->ui_node_detail == ViewDetail::MINIMALIST;

    switch (node()->type() )
    {
        case NodeType_VARIABLE_REF:
        {
            if ( minimalist )
                return "&";
            return node()->name();
        }
        case NodeType_VARIABLE:
        {
            if (minimalist)
                return "";
            auto variable = static_cast<const VariableNode *>( node() );
            return variable->get_type()->name();
        }
        case NodeType_OPERATOR:
        {
            return node()->name();
        }
        case NodeType_FUNCTION:
        {
            if ( minimalist )
                return "f(x)";
            return node()->name();
        }
        case NodeType_ENTRY_POINT:
        {
            if ( minimalist )
            {
                return node()->name().substr(0, 6); // 4 char for the icon
            }
            return node()->name();
        }
        case NodeType_BLOCK_IF:
        {
            if ( minimalist )
                return "?";
            return node()->name();
        }
        case NodeType_BLOCK_FOR_LOOP:
        {
            if ( minimalist )
                return "for";
            return node()->name();
        }
        default:
        {
            if ( minimalist )
                return node()->name().substr(0, 3) + ".";
            return node()->name();
        }
    }

}

void NodeView::reset()
{
    if ( node() == nullptr )
    {
        return;
    }

    Config* cfg = get_config();

    // 1. Create Property views
    //-------------------------

    VERIFY(m_view_by_property.empty(), "Cannot be called twice");

    for (Property* property : node()->props() )
    {
        // Create view
        auto new_view = new PropertyView(property);
        add_child( new_view );

        switch ( node()->type() )
        {
            case NodeType_ENTRY_POINT:
            case NodeType_FUNCTION:
            case NodeType_OPERATOR:
            case NodeType_BLOCK_FOR_LOOP:
            case NodeType_BLOCK_IF:
            case NodeType_BLOCK_WHILE_LOOP:
                // hide THIS property
                if ( property->has_flags(PropertyFlag_IS_NODE_VALUE) )
                    new_view->state().visible = false;
        }

        // Indexing
        if (property == node()->value() )
        {
            m_value_view = new_view;
        }

        bool has_in  = node()->find_slot_by_property(property, SlotFlag_INPUT );
        bool has_out = node()->find_slot_by_property(property, SlotFlag_OUTPUT );

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

    // 2. Create a SlotView per slot
    //------------------------------

    for(auto* each : m_slot_views )
        delete each;
    m_slot_views.clear();

    static const std::unordered_map<SlotFlags, ShapeType> shape_per_type
    {
        { SlotFlag_TYPE_FLOW , ShapeType_RECTANGLE },
        { SlotFlag_TYPE_VALUE, ShapeType_CIRCLE },
    };

    static const std::unordered_map<SlotFlags, Vec2> align_per_type
    {
        {SlotFlag_INPUT  ,     TOP },
        {SlotFlag_OUTPUT ,     BOTTOM },
        {SlotFlag_FLOW_IN   ,  TOP_LEFT },
        {SlotFlag_FLOW_OUT   , BOTTOM_LEFT }
    };

    std::unordered_map<SlotFlags, u8_t> count_per_type
    {
        {SlotFlag_FLOW_OUT   , 0 },
        {SlotFlag_FLOW_IN   ,  0 },
        {SlotFlag_INPUT  ,     0 },
        {SlotFlag_OUTPUT ,     0 }
    };

    // Create a view per slot
    for( Slot* slot : node()->slots() )
    {
        const Vec2&      alignment     = align_per_type.at(slot->type_and_order());
        const ShapeType& shape         = shape_per_type.at(slot->type());
        const u8_t       index         = count_per_type[slot->type_and_order()]++;

        auto* view = new SlotView( slot, alignment, shape, index, this->shape() );
        add_child( view );
    }

    // Make sure inputs/outputs are aligned with the property views (if present) and not the node's view.
    for(auto view : m_slot_views)
    {
        switch ( view->slot->type() )
        {
            case SlotFlag_TYPE_VALUE:
            {
                const PropertyView* property_view = find_property_view( view->property() );
                if ( property_view != nullptr && property_view->state().visible )
                    view->alignment_ref = &property_view->shape();
            }
        }
    }

    // Adjust some slot views
    switch ( node()->type() )
    {
        case NodeType_VARIABLE:
        {
            auto variable = static_cast<VariableNode*>( node() );
            if ( Slot* decl_out = variable->decl_out() )
            {
                if (SlotView *view = decl_out->view)
                {
                    view->alignment = LEFT;
                    view->update_direction_from_alignment();
                    view->alignment_ref = this->shape();
                }
            }
            break;
        }
        case NodeType_FUNCTION:
        {
            auto function = static_cast<FunctionNode*>( node() );
            if ( Slot* value_out = function->value_out() )
            {
                if (SlotView *view = value_out->view)
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
    set_color( &cfg->ui_node_fill_color[ node()->type()] );
}

void NodeView::arrange_recursively(bool _smoothly)
{
    for (auto each_input: get_adjacent(SlotFlag_INPUT) )
    {
        if ( !each_input->m_pinned )
            if (Utils::is_output_node_in_expression(each_input->node(), this->node()) )
                each_input->arrange_recursively();
    }

    if (node()->has_internal_scope() )
        for ( Node* node : node()->internal_scope()->child() )
            if ( NodeView* node_view = node->get_component<NodeView>() )
                    node_view->arrange_recursively();

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well-placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        update(float(1000));
    }

    m_pinned = false;
}

void NodeView::update(float dt)
{
    if(m_opacity != 1.0f)
        lerp(m_opacity, 1.0f, 10.0f * dt);

    for(SlotView* slot_view  : m_slot_views )
        slot_view->update( dt );
}

bool NodeView::draw()
{
    m_state.shape().draw_debug_info();

    if ( !m_state.visible )
        return false;

    if ( !node() )
        return false;

    Config*     cfg       = get_config();
	bool        changed   = false;

    m_hovered_slotview      = nullptr; // reset every frame
    m_last_clicked_slotview = nullptr; // reset every frame

    // Draw background slots (rectangles)
    for( SlotView* slot_view: m_slot_views )
        if ( slot_view->shape_type == ShapeType_RECTANGLE)
            draw_slot(slot_view);

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
    if ( m_state.selected )
    {
        border_color = cfg->ui_node_borderHighlightedColor;
    }
    else if ( Utils::is_instruction( node() ) )
    {
        border_color = cfg->ui_node_fill_color[NodeType_DEFAULT];
    }

    float border_width = cfg->ui_node_borderWidth;
    if( Utils::is_instruction( node() ) )
    {
        border_width *= cfg->ui_node_instructionBorderRatio;
    }

    DrawNodeRect(
            screen_rect,
            get_color( Color_FILL ),
            cfg->ui_node_borderColor,
            cfg->ui_node_shadowColor,
            border_color,
            m_state.selected,
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

    switch ( node()->type() )
    {
        case NodeType_OPERATOR:
            if ( Utils::is_unary_operator( node() ) )
                pre_label = get_label();
            else if ( Utils::is_binary_operator( node() ) )
                operator_label[0] = get_label();
            // else if (node->is_ternary_operator()
            break;
        default:
            pre_label = get_label();
            break;
        case NodeType_FUNCTION:
            pre_label = get_label();
            post_label = "";

            if ( cfg->ui_node_detail != ViewDetail::MINIMALIST)
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

        if ( node()->type() == NodeType_FUNCTION )
            if (Slot *slot_out = node()->value_out())
                if (SlotView *slot_view_out = slot_out->view)
                {
                    const float x = ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x * 0.5f;
                    const float y = shape()->pivot(BOTTOM, WORLD_SPACE).y;
                    slot_view_out->spatial_node().set_position({x, y}, WORLD_SPACE);
                    slot_view_out->direction = BOTTOM;
                }
    }

    // Draw the properties depending on node type
    if ( node()->type() != NodeType_OPERATOR )
    {
        changed |= PropertyView::draw_all(m_view_by_property_type[PropType_IN_STRICTLY], cfg->ui_node_detail);
        changed |= PropertyView::draw_all(m_view_by_property_type[PropType_INOUT_STRICTLY], cfg->ui_node_detail);
        changed |= PropertyView::draw_all(m_view_by_property_type[PropType_OUT_STRICTLY], cfg->ui_node_detail);
    }
    else
    {
        size_t i = 0;
        for( PropertyView* property_view : m_view_by_property_type[PropType_IN] )
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
    for( SlotView* slot_view: m_slot_views )
        if ( slot_view->shape_type == ShapeType_CIRCLE)
            draw_slot(slot_view);

	ImGui::PopStyleVar();
	ImGui::PopID();

    if ( changed )
        node()->set_flags(NodeFlag_IS_DIRTY );

    m_state.hovered = is_rect_hovered || m_hovered_slotview != nullptr;

	return changed;
}

void NodeView::DrawNodeRect(
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
        auto alpha   = sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
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

bool NodeView::is_inside(NodeView* _other, const Rect& _rect, Space _space)
{
	return Rect::contains(_rect, _other->shape()->rect(_space) );
}

bool NodeView::draw_as_properties_panel(NodeView *_view, bool* _show_advanced)
{
    bool changed = false;
    tools::Config* tools_cfg = tools::get_config();
    Node* node = _view->node();
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](PropertyView* _property_view) -> bool
    {
        Property*property = _property_view->get_property();
        // label (<name> (<type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s): ",
                property->name().c_str(),
                property->get_type()->name());

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("Source token:\n %s\n", property->token().json().c_str());
            ImGuiEx::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        return PropertyView::draw_input(_property_view, !_show_advanced, nullptr);
    };

    ImGui::Text("Name:       \"%s\"" , node->name().c_str());
    ImGui::Text("Class:      %s"     , node->get_class()->name());

    // Draw exposed input properties

    auto draw_properties = [&](const char* title, const std::vector<PropertyView*>& views) -> bool
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
    ImGui::Text("Other Propertie(s) (%zu)", node->get_components().size() );
    ImGui::Separator();
    changed |= draw_labeled_property_view( _view->m_value_view );
    ImGui::Separator();

    ImGui::Separator();
    ImGui::Text("Component(s) (%zu)", node->get_components().size() );
    ImGui::Separator();
    for ( NodeComponent* component : node->get_components() )
    {
        ImGui::PushID( component );
        const char* name = component->get_class()->name();
        if( ImGui::TreeNode( name ) )
        {
            if (component->get_class() == type::get<Physics>())
            {
                Physics *physics_component = static_cast<Physics *>( component );
                ImGui::Checkbox("On/Off", &physics_component->is_active());

                for (ViewConstraint& constraint: physics_component->constraints())
                {
                    ImGui::PushID(&constraint);
                    if (ImGui::TreeNode(constraint.name) )
                    {
                        ImGui::Checkbox("enabled", &constraint.enabled);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }
            else if (component->get_class() == type::get<Scope>())
            {
                Scope *scope = static_cast<Scope *>( component );
                if (ImGui::TreeNode("Node(s)"))
                {
                    for (Node *child: scope->child())
                    {
                        ImGui::BulletText("%s (class %s)", child->name().c_str(), child->get_class()->name());
                    }
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("VariableNode(s)"))
                {
                    for (VariableNode *variable: scope->variable())
                    {
                        std::string value = variable->value()->token().word_to_string();
                        ImGui::BulletText("%s (value: %s)", variable->name().c_str(), value.c_str());
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::Separator();

    ImGui::Separator();
    ImGui::Text("Slots");
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
                ImGui::BulletText("- %s", each_node->name().c_str());
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
            Scope* scope = node->scope();
            if (scope)
            {
                string128 label;
                Node* scope_owner = scope->node();
                label.append_fmt("%s %p (%s %p)", scope->name(), scope, scope_owner->name().c_str(), scope_owner);
                if ( ImGui::Button(label.c_str()) )
                    node->graph()->view()->set_selected({ scope_owner->get_component<NodeView>() });
            }
            else
            {
                ImGui::Text("nullptr");
            }

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("dirty");
            ImGui::TableNextColumn();
            ImGui::Text(node->has_flags(NodeFlag_IS_DIRTY) ? "yes" : "no");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("suffix token");
            ImGui::TableNextColumn();
            ImGui::Text("%s", node->suffix().json().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("can_be_instruction");
            ImGui::TableNextColumn();
            ImGui::Text("%i", Utils::can_be_instruction(node));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("is_instruction");
            ImGui::TableNextColumn();
            ImGui::Text("%i", Utils::is_instruction(node));

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
#endif // NDBL_DEBUG
    return changed;
}

void NodeView::constraint_to_rect(NodeView* _view, const Rect& _rect)
{
	
	if ( !NodeView::is_inside(_view, _rect ))
    {
        Rect shrinked_rect = _rect;
        shrinked_rect.expand( Vec2( -2, -2 ) ); // shrink

		auto view_rect = _view->shape()->rect();

		auto left  = _rect.min.x - view_rect.min.x;
		auto right = _rect.max.x - view_rect.max.x;
		auto up    = _rect.min.y - view_rect.min.y;
		auto down  = _rect.max.y - view_rect.max.y;

		     if ( left > 0 )  view_rect.translate_x(left );
		else if ( right < 0 ) view_rect.translate_x(right );
			 
			 if ( up > 0 )  view_rect.translate_y(up );
		else if ( down < 0 )view_rect.translate_y(down );

        _view->spatial_node().set_position(view_rect.center(), PARENT_SPACE);
	}

}

Rect NodeView::get_rect(Space space) const
{
    return m_state.shape().rect(space);
}

Rect NodeView::get_rect_ex(tools::Space space, NodeViewFlags flags) const
{
    if( (flags & NodeViewFlag_WITH_RECURSION) == 0 )
        return this->get_rect(space);

    std::vector<Rect> rects;

    if ( m_state.visible )
        rects.push_back( this->get_rect(space) );

    auto visit = [&](Node* node)
    {
        NodeView* view = node->get_component<NodeView>();
        if( !view )
            return;
        if( !view->m_state.visible )
            return;
        if(view->m_state.selected && (flags & NodeViewFlag_EXCLUDE_UNSELECTED) )
            return;
        if( view->m_pinned && (flags & NodeViewFlag_WITH_PINNED ) == 0 )
            return;
        if(Utils::is_output_node_in_expression(view->node(), this->node()) )
        {
            Rect rect = view->get_rect_ex(space, flags);
            rects.push_back( rect );
        }
    };

    if (node()->has_internal_scope() )
        for (Node* node : node()->internal_scope()->child() )
            visit(node);

    for (Node* node : node()->inputs() )
        visit(node);

    Rect result = Rect::bbox(&rects);

#if DEBUG_DRAW
    Rect screen_rect = result;
    screen_rect.translate(get_pos(space) - get_pos(PARENT_SPACE) );
    ImGuiEx::DebugRect(screen_rect.min, screen_rect.max, IM_COL32( 0, 255, 0, 60 ), 2 );
#endif

    return result;
}

Rect NodeView::get_rect(
    const std::vector<NodeView *> &_views,
    Space space,
    NodeViewFlags flags
)
{
    Rect result;
    for (size_t i = 0; i < _views.size(); ++i)
    {
        Rect rect = _views[i]->get_rect_ex(space, flags);
        if ( !result.has_area() )
            result = rect;
        else
            result = Rect::merge(result, rect);
    }
    return result;
}

std::vector<Rect> NodeView::get_rects(const std::vector<NodeView*>& _in_views, Space space, NodeViewFlags flags)
{
    std::vector<Rect> rects;
    rects.reserve(_in_views.size());
    size_t i = 0;
    while( i < _in_views.size() )
    {
        rects[i] = _in_views[i]->get_rect_ex(space, flags);
        ++i;
    }
    return std::move( rects );
}

void NodeView::set_expanded_rec(bool _expanded)
{
    set_expanded(_expanded);

    if ( !node()->has_internal_scope() )
        return;

    for(Node* child : node()->internal_scope()->child() )
    {
        child->get_component<NodeView>()->set_expanded_rec(_expanded);
    }
}

void NodeView::set_expanded(bool _expanded)
{
    m_expanded = _expanded;
    set_inputs_visible(_expanded, true);
    set_children_visible(_expanded, true);
}

void NodeView::set_inputs_visible(bool _visible, bool _recursive)
{
    set_adjacent_visible( SlotFlag_INPUT, _visible, NodeViewFlag_WITH_RECURSION * _recursive );
}

void NodeView::set_children_visible(bool visible, bool recursively)
{
    if ( !node()->has_internal_scope() )
        return;

    std::set<Scope*> scopes;
    Scope::get_descendent(scopes, node()->internal_scope(), 1 );

    for(Scope* scope : scopes)
    {
        for (Node* child: scope->child())
        {
            NodeView *child_view = child->get_component<NodeView>();
            child_view->m_state.visible = visible;
        }
    }
}

void NodeView::set_adjacent_visible(SlotFlags slot_flags, bool _visible, NodeViewFlags node_flags)
{
    bool has_not_output = node()->outputs().empty();
    for( auto each_child_view : get_adjacent(slot_flags) )
    {
        if( _visible || has_not_output || Utils::is_output_node_in_expression(each_child_view->node(),
                                                                              this->node()) )
        {
            if ( (node_flags & NodeViewFlag_WITH_RECURSION) && each_child_view->m_expanded ) // propagate only if expanded
            {
                each_child_view->set_children_visible(_visible, true);
                each_child_view->set_inputs_visible(_visible, true);
            }
            each_child_view->m_state.visible = _visible;
        }
    }
}

NodeView* NodeView::substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive)
{
    if( _view == nullptr )
    {
        return _view;
    }

    if( _view->m_state.visible )
    {
        return _view;
    }

    if ( _recursive )
        if( Scope* scope = _view->node()->scope() )
            if (NodeView* parent_view = scope->node()->get_component<NodeView>() )
                return parent_view->visible() ? parent_view
                                              : substitute_with_parent_if_not_visible(parent_view, _recursive);

    return nullptr;
}

std::vector<NodeView*> NodeView::substitute_with_parent_if_not_visible(const std::vector<NodeView*>& _in, bool _recursive)
{
    std::vector<NodeView*> out;
    out.reserve(_in.size()); // Wort but more probable case
    for(auto each : _in)
    {
        auto each_or_substitute = NodeView::substitute_with_parent_if_not_visible(each, _recursive);
        if (each_or_substitute)
        {
            out.push_back(each_or_substitute);
        }
    }
    return std::move(out);
};

std::vector<NodeView*> NodeView::get_adjacent(SlotFlags flags) const
{
    return Utils::adjacent_components<NodeView>(node(), flags);
}

void NodeView::set_color( const Vec4* _color, ColorType _type )
{
    ASSERT(_color != nullptr);
    m_colors[_type] = _color;
}

Vec4 NodeView::get_color( ColorType _type ) const
{
     auto* color = m_colors[_type];
     VERIFY(color != nullptr, "Did you called set_color(...) ?");
     return *color;
}

GraphView *NodeView::graph_view() const
{
    ASSERT(node()->graph() != nullptr);
    return node()->graph()->view();
}

void NodeView::draw_slot(SlotView* slot_view)
{
    if( slot_view->draw() )
        m_last_clicked_slotview = slot_view;

    if( slot_view->state().hovered )
    {
        m_hovered_slotview = slot_view; // last wins
    }
}

void NodeView::add_child(PropertyView* view)
{
    spatial_node().add_child( &view->spatial_node() );
    view->spatial_node().set_position({0.f, 0.f}, PARENT_SPACE);
}

void NodeView::add_child(SlotView* view)
{
    spatial_node().add_child( &view->spatial_node() );
    view->spatial_node().set_position({0.f, 0.f}, PARENT_SPACE);
    m_slot_views.push_back( view );
}

PropertyView *NodeView::find_property_view(const Property* property)
{
    auto found = m_view_by_property.find(property );
    if (found != m_view_by_property.end() )
        return found->second;
    return nullptr;
}

void NodeView::reset_all_properties()
{
    for( auto& [_, property_view] : m_view_by_property )
        property_view->reset();
}
