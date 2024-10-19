#include "NodeView.h"

#include <algorithm> // for std::max
#include <cmath> // for sinus
#include <vector>

#include "tools/core/math.h"
#include "ndbl/core/GraphUtil.h"
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

REFLECT_STATIC_INIT
{
    type::Initializer<NodeView>("NodeView")
        .extends<NodeComponent>();
}

constexpr Vec2 DEFAULT_SIZE             = Vec2(10.0f, 35.0f);
constexpr Vec2 DEFAULT_POS              = Vec2(500.0f, -1.0f);
constexpr Vec4 DEFAULT_COLOR            = Vec4(1.f, 0.f, 0.f);
constexpr bool PIXEL_PERFECT            = true; // round positions for drawing only

NodeView::NodeView()
    : NodeComponent()
    , m_colors({&DEFAULT_COLOR})
    , m_opacity(1.0f)
    , m_expanded(true)
    , m_pinned(false)
    , m_value_view(nullptr)
    , m_property_views__all()
    , m_hovered_slotview(nullptr)
    , m_last_clicked_slotview(nullptr)
    , m_view_state(DEFAULT_SIZE.x, DEFAULT_SIZE.y)
{
}

NodeView::~NodeView()
{
    for(auto& [_, each] : m_property_views__all )
        delete each;

    for(auto* each : m_slot_views )
        delete each;
}

std::string NodeView::get_label()
{
    Config* cfg = get_config();
    if (cfg->ui_node_detail == ViewDetail::MINIMALIST )
    {
        // I always add an ICON_FA at the beginning of any node label string (encoded in 4 bytes)
        return m_short_label;
    }
    return m_label;
}

void NodeView::set_owner(Node* node)
{
    NodeComponent::set_owner(node);

    if( node == nullptr )
    {
        return;
    }

    Config* cfg = get_config();

    // 1. Create Property views
    //-------------------------

    // Reserve
    for(auto& [_, property_view] : m_property_views__all)
        delete property_view;

    m_property_views__all.clear();
    m_property_views__out_strictly.clear();
    m_property_views__inout_strictly.clear();
    m_property_views__in_strictly.clear();
    m_property_views__in.clear();
    m_property_views__out.clear();

    for (Property* property : node->props() )
    {
        switch ( node->type() )
        {
            case NodeType_FUNCTION:
            case NodeType_OPERATOR:
            case NodeType_BLOCK_FOR_LOOP:
            case NodeType_BLOCK_CONDITION:
            case NodeType_BLOCK_SCOPE:
            case NodeType_BLOCK_WHILE_LOOP:
                // we don't need to actually see this view for now
                if ( property->has_flags(PropertyFlag_IS_THIS) )
                    continue;
        }

        // Create view
        auto new_view = new PropertyView(property);
        add_child( new_view );

        // Indexing
        if ( property == node->value() )
        {
            m_value_view = new_view;
        }

        bool has_in  = get_node()->find_slot_by_property(property, SlotFlag_INPUT );
        bool has_out = get_node()->find_slot_by_property(property, SlotFlag_OUTPUT );

        if ( has_in)
            m_property_views__in.push_back(new_view);
        if ( has_out)
            m_property_views__out.push_back(new_view);

        if ( has_in && has_out )
            m_property_views__inout_strictly.push_back(new_view);
        else if ( has_in )
            m_property_views__in_strictly.push_back(new_view);
        else if ( has_out )
            m_property_views__out_strictly.push_back(new_view);

        m_property_views__all.emplace(property, new_view);
    }

    // 2. Create a SlotView per slot
    //------------------------------

    for(auto* each : m_slot_views )
        delete each;
    m_slot_views.clear();

    static const std::unordered_map<SlotFlags, ShapeType> shape_per_type
    {
        { SlotFlag_TYPE_CODEFLOW    , ShapeType_RECTANGLE },
        { SlotFlag_TYPE_VALUE       , ShapeType_CIRCLE },
        { SlotFlag_TYPE_HIERARCHICAL, ShapeType_NONE },
    };

    static const std::unordered_map<SlotFlags, Vec2> align_per_type
    {
        { SlotFlag_INPUT  , TOP },
        { SlotFlag_OUTPUT , BOTTOM },
        { SlotFlag_PREV   , TOP_LEFT },
        { SlotFlag_NEXT   , BOTTOM_LEFT }
    };

    std::unordered_map<SlotFlags, u8_t> count_per_type
    {
        { SlotFlag_NEXT   , 0 },
        { SlotFlag_PREV   , 0 },
        { SlotFlag_INPUT  , 0 },
        { SlotFlag_OUTPUT , 0 }
    };

    // Create a view per slot
    for(Slot* slot : get_node()->slots() )
    {
        // We don't want to see hierarchical slots
        if ( slot->type() == SlotFlag_TYPE_HIERARCHICAL )
            continue;

        const Vec2&      alignment     = align_per_type.at(slot->type_and_order());
        const ShapeType& shape         = shape_per_type.at(slot->type());
        const u8_t       index         = count_per_type[slot->type_and_order()]++;

        auto* view = new SlotView( slot, alignment, shape, index, this->box() );
        add_child( view );
    }

    // Make sure inputs/outputs are aligned with the property views (if present) and not the node's view.
    for(auto view : m_slot_views)
    {
        switch ( view->slot().type() )
        {
            case SlotFlag_TYPE_VALUE:
            {
                const PropertyView* property_view = find_property_view( view->property() );
                if ( property_view != nullptr )
                    view->set_align_ref( property_view->box() );
            }
        }
    }

    // Adjust some slot views
    switch ( node->type() )
    {
        case NodeType_VARIABLE:
        {
            auto variable = static_cast<VariableNode*>( node );
            if ( Slot* decl_out = variable->decl_out() )
            {
                if (SlotView *view = decl_out->view())
                {
                    view->set_align( LEFT );
                    view->set_align_ref( this->box() );
                }
            }
            break;
        }
        case NodeType_FUNCTION:
        {
            auto function = static_cast<FunctionNode*>( node );
            if ( Slot* value_out = function->value_out() )
            {
                if (SlotView *view = value_out->view())
                {
                    view->set_align({-0.75, 1.f}); // bottom-left, with a margin
                    view->set_align_ref( this->box() );
                }
            }
            break;
        }
    }

    // 3. Update label
    //----------------

    update_labels_from_name(node );
    node->on_name_change().connect([=](Node* _node)
    {
        this->update_labels_from_name(_node);
    });

    // 4. Update fill color
    //---------------------

    // note: We pass color by address to be able to change the color dynamically
    set_color( &cfg->ui_node_fill_color[node->type()] );
}

void NodeView::update_labels_from_name(const Node* _node)
{
    // Label
    // For a variable, label must be the type
    if ( _node->type() == NodeType_VARIABLE )
        m_label = reinterpret_cast<const VariableNode *>(_node)->get_type()->get_name();
    else
        m_label = _node->name();

    // Short label
    constexpr size_t label_max_length = 10;
    if ( m_label.size() <= label_max_length )
        m_short_label = m_label;
    else
        m_short_label = m_label.substr(0, label_max_length) + "..";
}

void NodeView::arrange_recursively(bool _smoothly)
{
    for (auto each_input: get_adjacent(SlotFlag_INPUT) )
    {
        if ( !each_input->m_pinned && each_input->get_node()->should_be_constrain_to_follow_output( this->get_node() ))
        {
            each_input->arrange_recursively();
        }
    }

    for (auto each_child: get_adjacent(SlotFlag_CHILD)  )
    {
        each_child->arrange_recursively();
    }

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well-placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        update(float(1000));
    }

    m_pinned = false;
}

bool NodeView::update(float _deltaTime)
{
    if(m_opacity != 1.0f)
        lerp(m_opacity, 1.0f, 10.0f * _deltaTime);

    for(SlotView* slot_view  : m_slot_views )
        slot_view->update( _deltaTime );

	return true;
}

bool NodeView::draw()
{
    m_view_state.box.draw_debug_info();

    if ( !m_view_state.visible )
        return false;

    Config*     cfg       = get_config();
	bool        changed   = false;
    Node*       node      = get_node();

    ASSERT(node != nullptr);

    m_hovered_slotview      = nullptr; // reset every frame
    m_last_clicked_slotview = nullptr; // reset every frame

    // Draw background slots (rectangles)
    for( SlotView* slot_view: m_slot_views )
        if (slot_view->shape() == ShapeType_RECTANGLE)
            draw_slot(slot_view);

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
    Rect screen_rect = get_rect(WORLD_SPACE);

    if ( PIXEL_PERFECT )
    {
        screen_rect.min.round();
        screen_rect.max.round();
    }
    ImGui::SetCursorScreenPos( screen_rect.top_left() ); // start from th top left corner
	ImGui::PushID(this);


	// Draw the background of the Group
    Vec4 border_color = cfg->ui_node_borderColor;
    if ( m_view_state.selected )
    {
        border_color = cfg->ui_node_borderHighlightedColor;
    }
    else if (node->is_instruction())
    {
        border_color = cfg->ui_node_fill_color[NodeType_DEFAULT];
    }

    float border_width = cfg->ui_node_borderWidth;
    if( node->is_instruction() )
    {
        border_width *= cfg->ui_node_instructionBorderRatio;
    }

    DrawNodeRect(
            screen_rect,
            get_color( Color_FILL ),
            cfg->ui_node_borderColor,
            cfg->ui_node_shadowColor,
            border_color,
            m_view_state.selected,
            5.0f,
            border_width );

    // Add an invisible just on top of the background to detect mouse hovering
	ImGui::SetCursorScreenPos(screen_rect.top_left());
    bool is_rect_hovered = !ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringRect(screen_rect.min, screen_rect.max);
    Vec2 new_screen_pos = screen_rect.top_left()
                          + Vec2{ cfg->ui_node_padding.x, cfg->ui_node_padding.y} // left and top padding.
                          ; // space for "this" left slot
    ImGui::SetCursorScreenPos(new_screen_pos);

	// Draw the window content
	//------------------------

    ImGui::BeginGroup();
    ImGui::Dummy({1.f, 1.f}); // Without this, drawing doesn't work (size issues), TODO: understand why and remove this

    // We currently don't need to see these property, unnecessary complexity
    // ImGui::SameLine(); draw_properties(m_property_views__out_strictly);

    std::string pre_label;
    std::vector<std::string> operator_label(1); // for binary (and ternary when implemented) operators
    std::string post_label;

    switch ( node->type() )
    {
        case NodeType_OPERATOR:
            if (node->is_unary_operator())
                pre_label = get_label();
            else if (node->is_binary_operator())
                operator_label[0] = get_label();
            break;
        default:
            pre_label = get_label();
            break;
        case NodeType_FUNCTION:
            pre_label = get_label() + "(";
            post_label = ")";
            break;
    }

    if ( !m_expanded )
        pre_label.append(" " ICON_FA_OBJECT_GROUP);

    // Draw the pre_label when necessary
    if ( !pre_label.empty() )
    {
        ImGui::SameLine(); ImGui::Text("%s", pre_label.c_str() );
    }

    // Draw the properties depending on node type
    if ( node->type() != NodeType_OPERATOR )
    {
        ImGui::SameLine(); PropertyView::draw_all(m_property_views__in_strictly,    cfg->ui_node_detail);
        ImGui::SameLine(); PropertyView::draw_all(m_property_views__inout_strictly, cfg->ui_node_detail);
        ImGui::SameLine(); PropertyView::draw_all(m_property_views__out_strictly,   cfg->ui_node_detail);
    }
    else
    {
        // TODO: handle ternary operators
        ImGui::SameLine();
        ImGui::BeginGroup();

        for(size_t i = 0; i < m_property_views__in.size(); i++)
        {
            ImGui::SameLine();
            PropertyView* property_view = m_property_views__in[i];
            changed |= property_view->draw( cfg->ui_node_detail );

            // draw inner label when necessary
            if ( i < operator_label.size() && !operator_label[i].empty() )
            {
                ImGui::SameLine(); ImGui::Text("%s", operator_label[i].c_str() );
            }
        }
        ImGui::EndGroup();
    }

    if ( !post_label.empty() )
    {
        ImGui::SameLine(); ImGui::Text("%s", post_label.c_str());
    }


    ImGui::EndGroup();

    // Ends the Window
    //----------------

    // Update box's size according to item's rect
    Vec2 new_size = ImGui::GetItemRectMax();
    new_size += Vec2{ cfg->ui_node_padding.z, cfg->ui_node_padding.w}; // right and bottom padding
    new_size -= screen_rect.top_left();
    new_size.x = std::max( 1.0f, new_size.x );
    new_size.y = std::max( 1.0f, new_size.y );

    box()->set_size(Vec2::round(new_size));

    // Draw foreground slots (circles)
    for( SlotView* slot_view: m_slot_views )
        if (slot_view->shape() == ShapeType_CIRCLE)
            draw_slot(slot_view);

	ImGui::PopStyleVar();
	ImGui::PopID();

    if ( changed )
        get_node()->set_flags( NodeFlag_IS_DIRTY );

    m_view_state.hovered = is_rect_hovered || m_hovered_slotview != nullptr;

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
	return Rect::contains(_rect, _other->box()->get_rect(_space) );
}

void NodeView::draw_as_properties_panel(NodeView *_view, bool* _show_advanced)
{
    tools::Config* tools_cfg = tools::get_config();
    Node* node = _view->get_node();
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](PropertyView* _property_view)
    {
        Property*property = _property_view->get_property();
        // label (<name> (<type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s): ",
                property->name().c_str(),
                property->get_type()->get_name());

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("Source token:\n %s\n", property->token().json().c_str());
            ImGuiEx::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool edited = PropertyView::draw_input(_property_view, !_show_advanced, nullptr);
        if ( edited )
            node->set_flags( NodeFlag_IS_DIRTY );
    };

    ImGui::Text("Name:       \"%s\"" , node->name().c_str());
    ImGui::Text("Class:      %s"     , node->get_class()->get_name());

    // Draw exposed input properties

    auto draw_properties = [&](const char* title, const std::vector<PropertyView*>& views)
    {
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
                draw_labeled_property_view( property_view );
                ImGui::Separator();
            }
        }
        ImGui::Unindent();
    };

    ImGui::Separator();
    draw_properties("Inputs(s)", _view->m_property_views__in_strictly);
    draw_properties("In/Out(s)", _view->m_property_views__inout_strictly);
    ImGui::Separator();
    draw_properties("Output(s)", _view->m_property_views__out_strictly);
    ImGui::Separator();

    if ( tools_cfg->runtime_debug )
    {
        ImGui::Text("DEBUG INFO:" );
        ImGui::Text("Suffix token:\n       %s\n" , node->suffix().json().c_str());
        ImGui::Text("can_be_instruction(): %i", node->can_be_instruction() );
        ImGui::Text("is_instruction():     %i", node->is_instruction() );
        // Draw exposed output properties
        if( ImGui::TreeNode("Other Properties") )
        {
            draw_labeled_property_view( _view->m_value_view );
            ImGui::TreePop();
        }

        // Components
        if( ImGui::TreeNode("Components") )
        {
            for (const NodeComponent* component : node->get_components() )
            {
                ImGui::BulletText("%s", component->get_class()->get_name());
            }
            ImGui::TreePop();
        }

        if( ImGui::TreeNode("Slots") )
        {
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

            draw_node_list("Inputs:"      , node->inputs() );
            draw_node_list("Outputs:"     , node->outputs() );
            draw_node_list("Predecessors:", node->predecessors() );
            draw_node_list("Successors:"  , node->successors() );
            draw_node_list("Children:"    , node->children() );

            ImGui::TreePop();
        }

        // Physics Component
        if( ImGui::TreeNode("Physics") )
        {
            auto* physics_component = node->get_component<Physics>();
            ImGui::Checkbox("On/Off", &physics_component->is_active);
            for(Physics::Constraint& constraint : physics_component->get_constraints())
            {
                if (ImGui::TreeNode(constraint.name))
                {
                    ImGui::Checkbox("enabled", &constraint.enabled);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        // Scope specific:
        if (Scope* scope = node->get_component<Scope>())
        {
            if( ImGui::TreeNode("Variables") )
            {
                for (VariableNode* variable : scope->variables())
                {
                    std::string value = variable->value()->token().word_to_string();
                    ImGui::BulletText("%s: %s", variable->name().c_str(), value.c_str() );
                }
                ImGui::TreePop();
            }
        }

        if( ImGui::TreeNode("Misc:") )
        {
            // dirty state
            ImGui::Separator();
            bool b = node->has_flags(NodeFlag_IS_DIRTY);
            ImGui::Checkbox("Is dirty ?", &b);

            // Parent graph
            {
                std::string parentName = "NULL";

                if (Graph* parent_graph = node->graph())
                {
                    parentName = "Graph";
                    parentName.append( parent_graph->is_dirty() ? " (dirty)" : "");
                }
                ImGui::Text("Parent graph is \"%s\"", parentName.c_str());
            }

            // Parent
            ImGui::Separator();
            {
                std::string parentName = "NULL";

                if (Node* parent = node->find_parent() )
                {
                    parentName = parent->name() + (parent->has_flags(NodeFlag_IS_DIRTY) ? " (dirty)" : "");
                }
                ImGui::Text("Parent node is \"%s\"", parentName.c_str());
            }
            ImGui::TreePop();
        }
    }
    ImGui::Separator();
}

void NodeView::constraint_to_rect(NodeView* _view, const Rect& _rect)
{
	
	if ( !NodeView::is_inside(_view, _rect ))
    {
        Rect shrinked_rect = _rect;
        shrinked_rect.expand( Vec2( -2, -2 ) ); // shrink

		auto view_rect = _view->box()->get_rect();

		auto left  = _rect.min.x - view_rect.min.x;
		auto right = _rect.max.x - view_rect.max.x;
		auto up    = _rect.min.y - view_rect.min.y;
		auto down  = _rect.max.y - view_rect.max.y;

		     if ( left > 0 )  view_rect.translate_x(left );
		else if ( right < 0 ) view_rect.translate_x(right );
			 
			 if ( up > 0 )  view_rect.translate_y(up );
		else if ( down < 0 )view_rect.translate_y(down );

        _view->xform()->set_pos( view_rect.center(), PARENT_SPACE );
	}

}

Rect NodeView::get_rect(Space space) const
{
    return m_view_state.box.get_rect(space);
}

Rect NodeView::get_rect_ex(tools::Space space, NodeViewFlags flags) const
{
    if( (flags & NodeViewFlag_WITH_RECURSION) == 0 )
        return this->get_rect(space);

    std::vector<Rect> rects;

    if ( m_view_state.visible )
        rects.push_back( this->get_rect(space) );

    auto visit = [&](NodeView* view)
    {
        if( !view )
            return;
        if( !view->m_view_state.visible )
            return;
        if(view->m_view_state.selected && (flags & NodeViewFlag_EXCLUDE_UNSELECTED) )
            return;
        if( view->m_pinned && (flags & NodeViewFlag_WITH_PINNED ) == 0 )
            return;
        if( view->get_node()->should_be_constrain_to_follow_output( this->get_node() ) )
        {
            Rect rect = view->get_rect_ex(space, flags);
            rects.push_back( rect );
        }
    };

    auto children = get_adjacent(SlotFlag_CHILD);
    std::for_each(children.begin(), children.end(), visit );

    auto inputs   = get_adjacent(SlotFlag_INPUT);
    std::for_each(inputs.begin()  , inputs.end()  , visit );

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
        if ( i == 0 )
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
    for(NodeView* each_child_view : get_adjacent(SlotFlag_CHILD) )
    {
        each_child_view->set_expanded_rec(_expanded);
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
    set_adjacent_visible( SlotFlag_INPUT, _visible, _recursive );
}

void NodeView::set_children_visible(bool _visible, bool _recursive)
{
    set_adjacent_visible( SlotFlag_CHILD, _visible, _recursive );
}

void NodeView::set_adjacent_visible(SlotFlags flags, bool _visible, bool _recursive)
{
    bool has_not_output = get_adjacent(SlotFlag_OUTPUT).empty();
    for( auto each_child_view : get_adjacent(flags) )
    {
        if( _visible || has_not_output || each_child_view->get_node()->should_be_constrain_to_follow_output( get_node() ) )
        {
            if ( _recursive && each_child_view->m_expanded ) // propagate only if expanded
            {
                each_child_view->set_children_visible(_visible, true);
                each_child_view->set_inputs_visible(_visible, true);
            }
            each_child_view->m_view_state.visible = _visible;
        }
    }
}

void NodeView::expand_toggle()
{
    set_expanded(!m_expanded);
}

NodeView* NodeView::substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive)
{
    if( _view == nullptr )
    {
        return _view;
    }

    if( _view->m_view_state.visible )
    {
        return _view;
    }

    Node* parent = _view->get_node()->find_parent();
    if ( !parent )
    {
        return _view;
    }

    NodeView* parent_view = parent->get_component<NodeView>();
    if ( !parent_view )
    {
        return _view;
    }

    if (  _recursive )
    {
        return substitute_with_parent_if_not_visible(parent_view, _recursive);
    }

    return parent_view;
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

void NodeView::expand_toggle_rec()
{
    return set_expanded_rec(!m_expanded);
}

std::vector<NodeView*> NodeView::get_adjacent(SlotFlags flags) const
{
    return GraphUtil::adjacent_components<NodeView>(get_node(), flags);
}

void NodeView::set_color( const Vec4* _color, ColorType _type )
{
    ASSERT(_color != nullptr)
    m_colors[_type] = _color;
}

Vec4 NodeView::get_color( ColorType _type ) const
{
     auto* color = m_colors[_type];
     VERIFY(color != nullptr, "Did you called set_color(...) ?");
     return *color;
}

GraphView *NodeView::get_graph() const
{
    ASSERT(get_node()->graph() != nullptr)
    return get_node()->graph()->get_view();
}

void NodeView::draw_slot(SlotView* slot_view)
{
    if( slot_view->draw() )
        m_last_clicked_slotview = slot_view;

    if( slot_view->is_hovered() )
    {
        m_hovered_slotview = slot_view; // last wins
    }
}


void NodeView::translate(const std::vector<NodeView*>& _views, const Vec2& delta)
{
    for (auto node_view : _views )
    {
        node_view->xform()->translate(delta);
    }
}

void NodeView::add_child(PropertyView* view)
{
    xform()->add_child( view->xform() );
    view->xform()->set_pos({0.f, 0.f}, PARENT_SPACE);
}

void NodeView::add_child(SlotView* view)
{
    xform()->add_child( view->xform() );
    view->xform()->set_pos({0.f, 0.f}, PARENT_SPACE);
    m_slot_views.push_back( view );
}

PropertyView *NodeView::find_property_view(const Property* property)
{
    auto found = m_property_views__all.find( property );
    if ( found != m_property_views__all.end() )
        return found->second;
    return nullptr;
}

