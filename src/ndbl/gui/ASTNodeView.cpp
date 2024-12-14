#include "ASTNodeView.h"

#include <algorithm> // for std::max
#include <glm/trigonometric.hpp> // for sinus
#include <vector>

#include "ndbl/core/ASTUtils.h"
#include "ndbl/core/ASTFunctionCall.h"
#include "ndbl/core/ASTLiteral.h"

#include "Config.h"
#include "PhysicsComponent.h"
#include "ASTNodePropertyView.h"
#include "GraphView.h"
#include "ASTNodeSlotView.h"
#include "tools/gui/Config.h"
#include "ndbl/core/Interpreter.h"
#include "tools/core/math.h"
#include "BoxShapeComponent.h"

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 0
#endif

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(ASTNodeView).extends<ComponentFor<ASTNode>>();
)

constexpr Vec4 DEFAULT_COLOR = Vec4(1.f, 0.f, 0.f);
#define PIXEL_PERFECT true // round positions for drawing only

ASTNodeView::ASTNodeView(
    tools::BoxShape2D*  shape
    )
    : ComponentFor<ASTNode>("View")
    , m_node(nullptr)
    , m_colors({&DEFAULT_COLOR})
    , m_opacity(1.0f)
    , m_expanded(true)
    , m_value_view(nullptr)
    , m_view_by_property()
    , m_hovered_slotview(nullptr)
    , m_last_clicked_slotview(nullptr)
    , m_view_state()
    , m_shape(shape)
    , m_spatial_node(shape->spatial_node())
{
    ASSERT(m_spatial_node);
    ASSERT(m_shape);

    CONNECT(ComponentFor::on_set_entity, &ASTNodeView::on_owner_init, this );
}

ASTNodeView::~ASTNodeView()
{
    for(auto& [_, each] : m_view_by_property )
    {
        m_spatial_node->remove_child( each->spatial_node() );
        delete each;
    }

    for(auto vector : m_view_by_property_type )
    {
        vector.clear();
    }

    for(auto* each : m_slot_views )
    {
        m_spatial_node->remove_child( each->spatial_node() );
        delete each;
    }
    m_slot_views.clear();
    m_hovered_slotview      = nullptr;
    m_last_clicked_slotview = nullptr;
}

std::string ASTNodeView::get_label()
{
    Config* cfg = get_config();

    bool minimalist = cfg->ui_node_detail == ViewDetail::MINIMALIST;

    switch (m_node->type() )
    {
        case ASTNodeType_VARIABLE_REF:
        {
            if ( minimalist )
                return "&";
            return m_node->name();
        }
        case ASTNodeType_VARIABLE:
        {
            if (minimalist)
                return "";
            auto variable = static_cast<const ASTVariable *>( m_node );
            return variable->get_type()->name();
        }
        case ASTNodeType_OPERATOR:
        {
            return m_node->name();
        }
        case ASTNodeType_FUNCTION:
        {
            if ( minimalist )
                return "f(x)";
            return m_node->name();
        }
        case ASTNodeType_ENTRY_POINT:
        {
            if ( minimalist )
            {
                return m_node->name().substr(0, 6); // 4 char for the icon
            }
            return m_node->name();
        }
        case ASTNodeType_BLOCK_IF:
        {
            if ( minimalist )
                return "?";
            return m_node->name();
        }
        case ASTNodeType_BLOCK_FOR_LOOP:
        {
            if ( minimalist )
                return "for";
            return m_node->name();
        }
        default:
        {
            if ( minimalist )
                return m_node->name().substr(0, 3) + ".";
            return m_node->name();
        }
    }

}

void ASTNodeView::on_owner_init(ASTNode* node)
{
    m_node = node;

    Config* cfg = get_config();

    // 1. Create Property views
    //-------------------------

    VERIFY(m_view_by_property.empty(), "Cannot be called twice");

    for (ASTNodeProperty* property : m_node->props() )
    {
        // Create view
        auto new_view = new ASTNodePropertyView(property);
        add_child( new_view );

        switch ( m_node->type() )
        {
            case ASTNodeType_ENTRY_POINT:
            case ASTNodeType_FUNCTION:
            case ASTNodeType_OPERATOR:
            case ASTNodeType_BLOCK_FOR_LOOP:
            case ASTNodeType_BLOCK_IF:
            case ASTNodeType_BLOCK_WHILE_LOOP:
                // hide THIS property
                if ( property->has_flags(PropertyFlag_IS_NODE_VALUE) )
                    new_view->state()->set_visible(false);
        }

        // Indexing
        if (property == m_node->value() )
        {
            m_value_view = new_view;
        }

        bool has_in  = m_node->find_slot_by_property(property, SlotFlag_INPUT );
        bool has_out = m_node->find_slot_by_property(property, SlotFlag_OUTPUT );

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
    for( ASTNodeSlot* slot : m_node->slots() )
    {
        const Vec2&      alignment     = align_per_type.at(slot->type_and_order());
        const ShapeType& shape         = shape_per_type.at(slot->type());
        const u8_t       index         = count_per_type[slot->type_and_order()]++;

        auto* view = new ASTNodeSlotView(slot, alignment, shape, index, this->shape() );
        add_child( view );
    }

    // Make sure inputs/outputs are aligned with the property views (if present) and not the node's view.
    for(auto view : m_slot_views)
    {
        switch ( view->slot->type() )
        {
            case SlotFlag_TYPE_VALUE:
            {
                const ASTNodePropertyView* property_view = find_property_view(view->property() );
                if ( property_view != nullptr && property_view->state()->visible() )
                    view->alignment_ref = property_view->shape();
            }
        }
    }

    // Adjust some slot views
    switch ( m_node->type() )
    {
        case ASTNodeType_VARIABLE:
        {
            auto variable = static_cast<ASTVariable*>( m_node );
            if ( ASTNodeSlot* decl_out = variable->decl_out() )
            {
                if (ASTNodeSlotView *view = decl_out->view)
                {
                    view->alignment = LEFT;
                    view->update_direction_from_alignment();
                    view->alignment_ref = this->shape();
                }
            }
            break;
        }
        case ASTNodeType_FUNCTION:
        {
            auto function = static_cast<ASTFunctionCall*>( m_node );
            if ( ASTNodeSlot* value_out = function->value_out() )
            {
                if (ASTNodeSlotView *view = value_out->view)
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
    set_color( &cfg->ui_node_fill_color[ m_node->type()] );
}

void ASTNodeView::arrange_recursively(bool _smoothly)
{
    for (auto each_input: get_adjacent(SlotFlag_INPUT) )
    {
        if ( !each_input->m_view_state.pinned() )
            if (ASTUtils::is_output_node_in_expression(each_input->m_node, m_node ) )
                each_input->arrange_recursively();
    }

    if (ASTScope* internal_scope = m_node->internal_scope() )
        for ( ASTNode* _node : internal_scope->backbone() )
            if ( auto* _node_view = _node->components()->get<ASTNodeView>() )
                    _node_view->arrange_recursively();

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well-placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        update(float(1000));
    }

    m_view_state.set_pinned(false);
}

void ASTNodeView::update(float dt)
{
    if(m_opacity != 1.0f)
        tools::clamped_lerp(m_opacity, 1.0f, 10.0f * dt);

    for(ASTNodeSlotView* slot_view  : m_slot_views )
        slot_view->update( dt );
}

bool ASTNodeView::draw()
{
    m_shape->draw_debug_info();

    if ( !m_view_state.visible() )
        return false;

    if ( !m_node )
        return false;

    Config*     cfg       = get_config();
	bool        changed   = false;

    m_hovered_slotview      = nullptr; // reset every frame
    m_last_clicked_slotview = nullptr; // reset every frame

    // Draw background slots (rectangles)
    for( ASTNodeSlotView* slot_view: m_slot_views )
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
    if ( m_view_state.selected() )
    {
        border_color = cfg->ui_node_borderHighlightedColor;
    }
    else if ( ASTUtils::is_instruction(m_node ) )
    {
        border_color = cfg->ui_node_fill_color[ASTNodeType_DEFAULT];
    }

    float border_width = cfg->ui_node_borderWidth;
    if( ASTUtils::is_instruction(m_node ) )
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

    switch ( m_node->type() )
    {
        case ASTNodeType_OPERATOR:
            if ( ASTUtils::is_unary_operator(m_node ) )
                pre_label = get_label();
            else if ( ASTUtils::is_binary_operator(m_node ) )
                operator_label[0] = get_label();
            // else if (node->is_ternary_operator()
            break;
        default:
            pre_label = get_label();
            break;
        case ASTNodeType_FUNCTION:
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

        if (m_node->type() == ASTNodeType_FUNCTION )
            if (ASTNodeSlot *slot_out = m_node->value_out())
                if (ASTNodeSlotView *slot_view_out = slot_out->view)
                {
                    const float x = ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x * 0.5f;
                    const float y = shape()->pivot(BOTTOM, WORLD_SPACE).y;
                    slot_view_out->spatial_node()->set_position({x, y}, WORLD_SPACE);
                    slot_view_out->direction = BOTTOM;
                }
    }

    // Draw the properties depending on node type
    if (m_node->type() != ASTNodeType_OPERATOR )
    {
        changed |= ASTNodePropertyView::draw_all(m_view_by_property_type[PropType_IN_STRICTLY], cfg->ui_node_detail);
        changed |= ASTNodePropertyView::draw_all(m_view_by_property_type[PropType_INOUT_STRICTLY], cfg->ui_node_detail);
        changed |= ASTNodePropertyView::draw_all(m_view_by_property_type[PropType_OUT_STRICTLY], cfg->ui_node_detail);
    }
    else
    {
        size_t i = 0;
        for( ASTNodePropertyView* property_view : m_view_by_property_type[PropType_IN] )
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
    for( ASTNodeSlotView* slot_view: m_slot_views )
        if ( slot_view->shape_type == ShapeType_CIRCLE)
            draw_slot(slot_view);

	ImGui::PopStyleVar();
	ImGui::PopID();

    if ( changed )
        m_node->set_flags(ASTNodeFlag_IS_DIRTY );

    const bool _hovered = is_rect_hovered || m_hovered_slotview != nullptr;
    m_view_state.set_hovered(_hovered );

	return changed;
}

void ASTNodeView::DrawNodeRect(
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

bool ASTNodeView::is_inside(ASTNodeView* _other, const Rect& _rect, Space _space)
{
	return Rect::contains(_rect, _other->shape()->rect(_space) );
}

bool ASTNodeView::draw_as_properties_panel(ASTNodeView *_view, bool* _show_advanced)
{
    bool changed = false;
    ASTNode* node = _view->m_node;
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](ASTNodePropertyView* _property_view) -> bool
    {
        ASTNodeProperty*property = _property_view->get_property();
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
        return ASTNodePropertyView::draw_input(_property_view, !_show_advanced, nullptr);
    };

    ImGui::Text("Name:       \"%s\"" , node->name().c_str());
    ImGui::Text("Class:      %s"     , node->get_class()->name());

    // Draw exposed input properties

    auto draw_properties = [&](const char* title, const std::vector<ASTNodePropertyView*>& views) -> bool
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
    ImGui::Text("Component(s) (%zu)", node->components()->size() );
    ImGui::Separator();
    for (ComponentFor<ASTNode>* component : *node->components() )
    {
        if( ImGui::TreeNode(component, "Component %s", component->name().c_str() ) )
        {
            if ( component != *node->components()->begin() )
                ImGui::Separator();

            if ( component->get_class() == type::get<PhysicsComponent>())
            {
                auto* physics_component = reinterpret_cast<PhysicsComponent*>( component );

                ImGui::Checkbox("On/Off", &physics_component->is_active());

                for (ViewConstraint& constraint: physics_component->constraints())
                {
                    if (ImGui::TreeNode(&constraint, "%s", constraint.name) )
                    {
                        ImGui::Checkbox("enabled", &constraint.enabled);
                        ImGui::TreePop();
                    }
                }
            }
            else if (component->get_class() == type::get<ASTScope>())
            {
                ASTScopeView::ImGuiTreeNode_ASTScope("ASTScope", static_cast<ASTScope *>( component ));
            }
            ImGui::TreePop();
        }
    }
    ImGui::Separator();

    ImGui::Separator();
    ImGui::Text("Slots");
    ImGui::Separator();
    auto draw_node_list = [](const char *label, const std::vector<ASTNode*> _nodes )
        {
            if( !ImGui::TreeNode(label) )
            {
                return;
            }

            if ( _nodes.empty() )
            {
                ImGui::BulletText( "None" );
            }

            for (const ASTNode* each_node : _nodes )
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
            ASTScope* scope = node->scope();
            if (scope)
            {
                string128 label;
                ASTNode* _node = scope->entity();
                label.append_fmt("%s %p (%s %p)", scope->name().c_str(), scope, _node->name().c_str(), _node);
                if ( ImGui::Button(label.c_str()) )
                {
                    GraphView* graph_view = node->graph()->components()->get<GraphView>();
                    ASSERT(graph_view);
                    graph_view->selection().clear();
                    graph_view->selection().append(_node->components()->get<ASTNodeView>() );
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
            ImGui::Text(node->has_flags(ASTNodeFlag_IS_DIRTY) ? "yes" : "no");

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("suffix token");
            ImGui::TableNextColumn();
            ImGui::Text("%s", node->suffix().json().c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("can_be_instruction");
            ImGui::TableNextColumn();
            ImGui::Text("%i", ASTUtils::can_be_instruction(node));

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("is_instruction");
            ImGui::TableNextColumn();
            ImGui::Text("%i", ASTUtils::is_instruction(node));

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
#endif // NDBL_DEBUG
    return changed;
}

void ASTNodeView::constraint_to_rect(ASTNodeView* _view, const Rect& _rect)
{
	
	if ( !ASTNodeView::is_inside(_view, _rect ))
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

        _view->m_spatial_node->set_position(view_rect.center(), PARENT_SPACE);
	}

}

Rect ASTNodeView::get_rect(Space space) const
{
    return m_shape->rect(space);
}

Rect ASTNodeView::get_rect_ex(tools::Space space, NodeViewFlags flags) const
{
    if( (flags & NodeViewFlag_WITH_RECURSION) == 0 )
        return this->get_rect(space);

    std::vector<Rect> rects;

    if ( m_view_state.visible() )
        rects.push_back( this->get_rect(space) );

    auto visit = [&](ASTNode* node)
    {
        auto* view = node->components()->get<ASTNodeView>();
        if( !view )
            return;
        if( !view->m_view_state.visible() )
            return;
        if( view->m_view_state.selected() && (flags & NodeViewFlag_EXCLUDE_UNSELECTED) )
            return;
        if( view->m_view_state.pinned() && (flags & NodeViewFlag_WITH_PINNED ) == 0 )
            return;
        if( ASTUtils::is_output_node_in_expression(view->m_node, this->node()) )
        {
            Rect rect = view->get_rect_ex(space, flags);
            rects.push_back( rect );
        }
    };

    if ( ASTScope* _internal_scope = m_node->internal_scope() )
        for (ASTNode* _node : _internal_scope->backbone() ) // TODO: use ASTScopeView's content_rect instead?
            visit(_node);

    for (ASTNode* _node : m_node->inputs() )
    {
        visit(_node);
    }

    Rect result = Rect::bbox(&rects);

#if DEBUG_DRAW
    Rect screen_rect = result;
    screen_rect.translate(get_pos(space) - get_pos(PARENT_SPACE) );
    ImGuiEx::DebugRect(screen_rect.min, screen_rect.max, IM_COL32( 0, 255, 0, 60 ), 2 );
#endif

    return result;
}

Rect ASTNodeView::get_rect(
    const std::vector<ASTNodeView *> &_views,
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

std::vector<Rect> ASTNodeView::get_rects(const std::vector<ASTNodeView*>& _in_views, Space space, NodeViewFlags flags)
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

void ASTNodeView::set_expanded_rec(bool _expanded)
{
    set_expanded(_expanded);

    if ( ASTScope* _internal_scope = m_node->internal_scope() )
        for( ASTNode* _node : _internal_scope->backbone() )
            if ( auto* view = _node->components()->get<ASTNodeView>() )
                view->set_expanded_rec(_expanded);
}

void ASTNodeView::set_expanded(bool _expanded)
{
    m_expanded = _expanded;
    set_inputs_visible(_expanded, true);
    set_children_visible(_expanded, true);
}

void ASTNodeView::set_inputs_visible(bool _visible, bool _recursive)
{
    set_adjacent_visible( SlotFlag_INPUT, _visible, NodeViewFlag_WITH_RECURSION * _recursive );
}

void ASTNodeView::set_children_visible(bool visible, bool recursively)
{
    if ( !m_node->has_internal_scope() )
        return;

    std::set<ASTScope*> scopes;
    ASTScope::get_descendent(scopes, m_node->internal_scope(), 1 );

    for(ASTScope* _scope : scopes)
        for (ASTNode* _child_node: _scope->backbone())
            if ( auto* view = _child_node->components()->get<ASTNodeView>() )
                view->state()->set_visible(visible );
}

void ASTNodeView::set_adjacent_visible(SlotFlags slot_flags, bool _visible, NodeViewFlags node_flags)
{
    bool has_not_output = m_node->outputs().empty();
    for( auto each_child_view : get_adjacent(slot_flags) )
    {
        if(_visible || has_not_output || ASTUtils::is_output_node_in_expression(each_child_view->m_node,
                                                                                this->node()) )
        {
            if ( (node_flags & NodeViewFlag_WITH_RECURSION) && each_child_view->m_expanded ) // propagate only if expanded
            {
                each_child_view->set_children_visible(_visible, true);
                each_child_view->set_inputs_visible(_visible, true);
            }
            each_child_view->m_view_state.set_visible(_visible );
        }
    }
}

ASTNodeView* ASTNodeView::substitute_with_parent_if_not_visible(ASTNodeView* _view, bool _recursive)
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
        if( ASTScope* scope = _view->m_node->scope() )
            if (ASTNodeView* parent_view = scope->entity()->components()->get<ASTNodeView>() )
                return parent_view->m_view_state.visible() ? parent_view
                                                      : substitute_with_parent_if_not_visible(parent_view, _recursive);

    return nullptr;
}

std::vector<ASTNodeView*> ASTNodeView::substitute_with_parent_if_not_visible(const std::vector<ASTNodeView*>& _in, bool _recursive)
{
    std::vector<ASTNodeView*> out;
    out.reserve(_in.size()); // Wort but more probable case
    for(auto each : _in)
    {
        auto each_or_substitute = ASTNodeView::substitute_with_parent_if_not_visible(each, _recursive);
        if (each_or_substitute)
        {
            out.push_back(each_or_substitute);
        }
    }
    return std::move(out);
};

std::vector<ASTNodeView*> ASTNodeView::get_adjacent(SlotFlags flags) const
{
    return ASTUtils::adjacent_components<ASTNodeView>(m_node, flags);
}

void ASTNodeView::set_color(const Vec4* _color, ColorType _type )
{
    ASSERT(_color != nullptr);
    m_colors[_type] = _color;
}

Vec4 ASTNodeView::get_color(ColorType _type ) const
{
     auto* color = m_colors[_type];
     VERIFY(color != nullptr, "Did you called set_color(...) ?");
     return *color;
}

void ASTNodeView::draw_slot(ASTNodeSlotView* slot_view)
{
    if( slot_view->draw() )
        m_last_clicked_slotview = slot_view;

    if( slot_view->state()->hovered() )
    {
        m_hovered_slotview = slot_view; // last wins
    }
}

void ASTNodeView::add_child(ASTNodePropertyView* view)
{
    m_spatial_node->add_child( view->spatial_node() );
    view->spatial_node()->set_position({0.f, 0.f}, PARENT_SPACE);
}

void ASTNodeView::add_child(ASTNodeSlotView* view)
{
    m_spatial_node->add_child( view->spatial_node() );
    view->spatial_node()->set_position({0.f, 0.f}, PARENT_SPACE);
    m_slot_views.push_back( view );
}

ASTNodePropertyView *ASTNodeView::find_property_view(const ASTNodeProperty* property)
{
    auto found = m_view_by_property.find(property );
    if (found != m_view_by_property.end() )
        return found->second;
    return nullptr;
}

void ASTNodeView::reset_all_properties()
{
    for( auto& [_, property_view] : m_view_by_property )
        property_view->reset();
}

