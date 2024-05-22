#include "NodeView.h"

#include <algorithm> // for std::max
#include <cmath> // for sinus
#include <vector>

#include "fw/core/math.h"
#include "nodable/core/GraphUtil.h"
#include "nodable/core/InvokableComponent.h"
#include "nodable/core/LiteralNode.h"


#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeViewConstraint.h"
#include "Physics.h"
#include "PropertyView.h"
#include "SlotView.h"
#include "fw/gui/Config.h"
#include "fw/gui/gui.h"
#include "gui.h"

using namespace ndbl;
using namespace fw;

REGISTER
{
    registration::push_class<NodeView>("NodeView")
        .extends<Component>()
        .extends<View>();
}

constexpr Vec2 DEFAULT_SIZE{10.0f, 35.0f};
constexpr Vec2 DEFAULT_POS{500.0f, -1.0f};

PoolID<NodeView>   NodeView::s_selected;
PoolID<NodeView>   NodeView::s_dragged;

// TODO: move those values into the configuration
NodeViewDetail     NodeView::s_view_detail                       = NodeViewDetail::Default;
const float        NodeView::s_property_input_size_min           = 10.0f;
const Vec2 NodeView::s_property_input_toggle_button_size(10.0, 25.0f);

NodeView::NodeView()
        : Component()
        , View()
        , m_colors({})
        , m_opacity(1.0f)
        , m_expanded(true)
        , m_pinned(false)
        , m_property_view_this(nullptr)
        , m_edition_enable(true)
        , m_property_views()
{
    box.pos(DEFAULT_POS);
    box.size(DEFAULT_SIZE);
}

NodeView::~NodeView()
{
    // deselect
    if ( s_selected == m_id ) s_selected.reset();
}

std::string NodeView::get_label()
{
    if (s_view_detail == NodeViewDetail::Minimalist )
    {
        // I always add an ICON_FA at the beginning of any node label string (encoded in 4 bytes)
        return m_short_label;
    }
    return m_label;
}

void NodeView::set_owner(PoolID<Node> node)
{
    Component::set_owner(node);

    if( node == PoolID<Node>::null )
    {
        return;
    }

    // 1. Create Property views
    //-------------------------

    // Reserve
    m_property_views.reserve( node->props.size() );

    for (Property& each_prop : node->props )
    {
        // Create view
        PropertyView& property_view = m_property_views.emplace_back( m_id, each_prop.id);

        // Indexing
        if ( each_prop.is_this() )
        {
            m_property_view_this = &property_view;
        }
        else if ( !m_owner->find_slot_by_property_id( each_prop.id, SlotFlag_OUTPUT ) )
        {
            m_property_views_with_input_only.push_back(&property_view);
        }
        else
        {
            m_property_views_with_output_or_inout.push_back(&property_view);
        }
    }

    // 2. Create SlotViews
    //--------------------
    // Create a SlotView per slot
    for(Slot& slot : m_owner->slots() )
    {
        Vec2 alignment;
        switch ( slot.static_flags() ) // type and order flags only
        {
            case SlotFlag_INPUT:  alignment.y = -0.5f; break;
            case SlotFlag_PREV:   alignment   = { -0.5f, -0.5f}; break;
            case SlotFlag_OUTPUT: alignment   = slot.get_property()->is_this() ? Vec2{-0.5f, 0.0f}
                                                                               : Vec2{ 0.0f, 0.5f}; break;
            case SlotFlag_NEXT:   alignment   = { -0.5f, 0.5f}; break;
            case SlotFlag_CHILD:
            case SlotFlag_PARENT: break; // won't be displayed, let's keep default alignment (0,0)
            default:
                FW_EXPECT(false, "unhandled slot flags")
        }

        m_slot_views.emplace_back( slot, alignment );
    }

    // 3. Update label
    //----------------

    PoolID<NodeView> id = m_id;
    update_labels_from_name(node.get());
    node->on_name_change.connect([=](PoolID<Node> _node)
    {
        id->update_labels_from_name(_node.get());
    });
}

void NodeView::update_labels_from_name(const Node* _node)
{
    // Label

    m_label.clear();
    m_short_label.clear();
    if ( auto* variable = cast<const VariableNode>(_node) )
    {
        m_label += variable->type()->get_name();
        m_label += " ";
    }
    m_label += _node->name;

    // Short label
    constexpr size_t label_max_length = 10;
    m_short_label = m_label.size() <= label_max_length ? m_label
                                                       : m_label.substr(0, label_max_length) + "..";
}

void NodeView::set_selected(PoolID<NodeView> new_selection)
{
    EventManager& event_manager = EventManager::get_instance();

    if( s_selected == new_selection ) return;

    EventPayload_NodeViewSelectionChange event{ new_selection, s_selected };
    event_manager.dispatch<Event_SelectionChange>(event);

    s_selected = new_selection;
}

PoolID<NodeView> NodeView::get_selected()
{
	return s_selected;
}

bool NodeView::is_any_dragged()
{
	return s_dragged.get() != nullptr;
}

PoolID<NodeView> NodeView::get_dragged()
{
	return s_dragged;
}

bool NodeView::is_selected(PoolID<NodeView> view)
{
	return s_selected == view;
}

const PropertyView* NodeView::get_property_view( ID<Property> _id )const
{
    return &m_property_views.at((u32_t)_id);
}

void NodeView::translate( Vec2 _delta, NodeViewFlags flags)
{
    View::translate(_delta);

	if ( !(flags & NodeViewFlag_RECURSIVELY) ) return;

    for(auto each_input: get_adjacent(SlotFlag_INPUT)  )
    {
        if( !each_input ) continue;
        if( each_input->m_pinned && flags & NodeViewFlag_IGNORE_PINNED ) continue;
        if( each_input->m_owner->should_be_constrain_to_follow_output( this->m_owner ) )
        {
            each_input->translate(_delta, flags);
        }
    }
}

void NodeView::arrange_recursively(bool _smoothly)
{
    for (auto each_input: get_adjacent(SlotFlag_INPUT) )
    {
        if ( !each_input->m_pinned && each_input->m_owner->should_be_constrain_to_follow_output( this->m_owner ))
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
    {
        lerp(m_opacity, 1.0f, 10.0f * _deltaTime);
    }
	return true;
}

bool NodeView::onDraw()
{
	bool        changed   = false;
    Node*       node      = m_owner.get();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    FW_ASSERT(node != nullptr);

    // Draw Node slots (in background)
    bool is_slot_hovered = false;
    {
        std::unordered_map<SlotFlags, int> count_by_flags{{SlotFlag_NEXT, 0}, {SlotFlag_PREV, 0}};
        for ( SlotView& slot_view : m_slot_views )
        {
            if( slot_view.slot().capacity() && slot_view.slot().type() == SlotFlag_TYPE_CODEFLOW && (node->is_instruction() || node->can_be_instruction() ) )
            {
                int& count = count_by_flags[slot_view.slot().static_flags()];
                Rect rect = get_slot_rect( slot_view, count );
                SlotView::draw_slot_rectangle( draw_list, slot_view, rect, m_edition_enable);
                is_slot_hovered |= ImGui::IsItemHovered();
                count++;
            }
        }
    }

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
    Rect screen_rect = rect( WORLD_SPACE );
    ImGui::SetCursorScreenPos(screen_rect.tl() ); // start from th top left corner
	ImGui::PushID(this);


	// Draw the background of the Group
    Vec4 border_color = g_conf->ui_node_borderColor;
    if ( is_selected( m_id ) )
    {
        border_color = g_conf->ui_node_borderHighlightedColor;
    }
    else if (node->is_instruction())
    {
        border_color = g_conf->ui_node_instructionColor;
    }

    float border_width = g_conf->ui_node_borderWidth;
    if( node->is_instruction() )
    {
        border_width *= g_conf->ui_node_instructionBorderRatio;
    }

    DrawNodeRect(
            screen_rect,
            get_color( Color_FILL ),
            g_conf->ui_node_borderColor,
            g_conf->ui_node_shadowColor,
            border_color,
            is_selected( m_id ),
            5.0f,
            border_width );

    // Add an invisible just on top of the background to detect mouse hovering
	ImGui::SetCursorScreenPos(screen_rect.tl() );
	ImGui::InvisibleButton("node", box.size());
    ImGui::SetItemAllowOverlap();
    Vec2 new_screen_pos = screen_rect.tl()
                          + Vec2{ g_conf->ui_node_padding.x, g_conf->ui_node_padding.y} // left and top padding.
                          + Vec2{ g_conf->ui_slot_radius, 0.0f}; // space for "this" left slot
    ImGui::SetCursorScreenPos(new_screen_pos);

    bool is_node_hovered = ImGui::IsItemHovered();

	// Draw the window content
	//------------------------

    ImGui::BeginGroup();
        std::string label = get_label().empty() ? " " : get_label();                        // ensure a 1 char width, to be able to grab it
        if ( !m_expanded )
        {
            // symbolize the fact node view is not expanded
            //abel.insert(0, "<<");
            label.append(" " ICON_FA_OBJECT_GROUP);
        }
        ImGuiEx::ShadowedText( Vec2(1.0f), g_conf->ui_node_borderHighlightedColor, label.c_str()); // text with a lighter shadow (encrust effect)

        ImGui::SameLine();

        ImGui::BeginGroup();

        // draw properties
        auto draw_property_lambda = [&](PropertyView* view) {
            ImGui::SameLine();
            changed |= _draw_property_view( view );
        };
        std::for_each( m_property_views_with_input_only.begin(), m_property_views_with_input_only.end(), draw_property_lambda);
        std::for_each( m_property_views_with_output_or_inout.begin(), m_property_views_with_output_or_inout.end(), draw_property_lambda);

        ImGui::EndGroup();
        ImGui::SameLine();
    ImGui::EndGroup();

    // Ends the Window
    //----------------

    // Update box's size according to item's rect
    Vec2 new_size = ImGui::GetItemRectMax();
    new_size += Vec2{ g_conf->ui_node_padding.z, g_conf->ui_node_padding.w}; // right and bottom padding
    new_size -= screen_rect.tl();
    new_size.x = std::max( 1.0f, new_size.x );
    new_size.y = std::max( 1.0f, new_size.y );

    box.size( Vec2::round(new_size) );

    // Draw Property in/out slots
    {
        for( auto& slot_view: m_slot_views )
        {
            if( slot_view.slot().has_flags(SlotFlag_TYPE_VALUE) )
            {
                Vec2 screen_pos = get_slot_pos(slot_view.slot());
                SlotView::draw_slot_circle( draw_list, slot_view, screen_pos, m_edition_enable );
                is_slot_hovered |= ImGui::IsItemHovered();
            }
        }
    }

    // Contextual menu (right click)
    if ( is_node_hovered && !is_slot_hovered && ImGui::IsMouseReleased(1))
    {
        ImGui::OpenPopup("NodeViewContextualMenu");
    }

    if (ImGui::BeginPopup("NodeViewContextualMenu"))
    {
        if( ImGui::MenuItem("Arrange"))
        {
            this->arrange_recursively();
        }

        ImGui::MenuItem("Pinned", "", &m_pinned, true);

		if ( ImGui::MenuItem("Expanded", "", &m_expanded, true) )
        {
		    set_expanded(m_expanded);
        }

        ImGui::Separator();

        if( ImGui::Selectable("Delete", !m_edition_enable ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
        {
            node->flagged_to_delete = true;
        }

        ImGui::EndPopup();
    }

	// Selection by mouse (left or right click)
	if ( is_node_hovered && (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)))
	{
        set_selected( m_id );
    }

	// Mouse dragging
	if (get_dragged() != m_id )
	{
		if( !get_dragged() && !SlotView::get_dragged() && ImGui::IsMouseDown(0) && is_node_hovered && ImGui::IsMouseDragPastThreshold(0))
        {
            s_dragged = m_id;
        }
	}
	else if ( ImGui::IsMouseReleased(0))
	{
        s_dragged.reset();
	}		

	// Collapse on/off
	if( is_node_hovered && ImGui::IsMouseDoubleClicked(0))
	{
        expand_toggle();
	}

	ImGui::PopStyleVar();
	ImGui::PopID();

    m_owner->dirty |= changed;

    is_hovered = is_node_hovered || is_slot_hovered;

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

bool NodeView::_draw_property_view(PropertyView* _view)
{
    bool            changed            = false;
    Property*       property           = _view->get_property();
    bool            is_defined         = property->value()->is_defined();
    const type* owner_type         = m_owner->get_type();
    VariableNode*   connected_variable = _view->get_connected_variable();

    /*
     * Handle input visibility
     */
    if ( _view->touched )
    {
        // When touched, we show the input if the value is defined (can be edited).
        _view->show_input &= is_defined;
    }
    else
    {
        // When untouched, it depends...

        // Always show literals (their property don't have input slot)
        _view->show_input |= owner_type->is<LiteralNode>();
        // Always show when defined in exhaustive mode
        _view->show_input |= is_defined && s_view_detail == NodeViewDetail::Exhaustive;
        // Always show when connected to a variable
        _view->show_input |= connected_variable != nullptr;
        // Shows variable property only if they are not connected (don't need to show anything, the variable name is already displayed on the node itself)
        _view->show_input |= is_defined && (owner_type->is<VariableNode>() || !m_owner->has_input_connected(property->id));
    }

    // input
    float input_size = NodeView::s_property_input_toggle_button_size.x;

    if ( _view->show_input )
    {
        bool limit_size = !property->get_type()->is<bool>();

        if ( limit_size )
        {
            // try to draw an as small as possible input field
            std::string str = connected_variable ? connected_variable->name : property->to<std::string>();
            input_size = 5.0f + std::max(ImGui::CalcTextSize(str.c_str()).x, NodeView::s_property_input_size_min);
            ImGui::PushItemWidth(input_size);
        }
        changed = NodeView::draw_property_view(_view, nullptr);

        if ( limit_size )
        {
            ImGui::PopItemWidth();
        }
    }
    else
    {
        ImGui::Button("", NodeView::s_property_input_toggle_button_size);

        if ( ImGui::IsItemClicked(0) )
        {
            _view->show_input = !_view->show_input;
            _view->touched = true;
        }
    }

    if ( ImGuiEx::BeginTooltip() )
    {
        ImGui::Text("%s %s\n", property->get_type()->get_name(), property->get_name().c_str());

        std::string  source_code;
        if( property->get_type()->is<PoolID<Node>>() || m_owner->find_slot_by_property_id( property->id, SlotFlag_OUTPUT ))
        {
            source_code = Nodlang::get_instance().serialize_node( source_code, m_owner );
        }
        else
        {
            source_code = Nodlang::get_instance().serialize_property(source_code, property );
        }

        ImGui::Text("source: \"%s\"", source_code.c_str());

        ImGuiEx::EndTooltip();
    }

    // memorize property view rect (screen space)
    // enlarge rect to fit node_view top/bottom
    _view->screen_rect = {
            Vec2{ImGui::GetItemRectMin().x, rect( WORLD_SPACE ).min.y} ,
            Vec2{ImGui::GetItemRectMax().x, rect( WORLD_SPACE ).max.y}
    };
    ImGuiEx::DebugCircle( _view->screen_rect.center(), 2.5f, ImColor(0,0,0));

    return changed;
}

bool NodeView::draw_property_view(PropertyView* _view, const char* _override_label)
{
    bool      changed  = false;
    Property* property = _view->get_property();

    // Create a label (everything after ## will not be displayed)
    std::string label;
    if ( _override_label != nullptr )
    {
        label.append(_override_label);
    }
    else
    {
        label.append("##" + property->get_name());
    }

    auto input_text_flags = ImGuiInputTextFlags_None;

    if( const VariableNode* variable = _view->get_connected_variable() ) // if is a ref to a variable, we just draw variable name
    {
        char str[255];
        snprintf(str, 255, "%s", variable->name.c_str() );

        ImGui::PushStyleColor(ImGuiCol_FrameBg, variable->get_component<NodeView>()->get_color(Color_FILL) );
        ImGui::InputText(label.c_str(), str, 255, input_text_flags );
        ImGui::PopStyleColor();

    }
    else if( !property->value()->is_initialized() )
    {
        ImGui::LabelText(label.c_str(), "uninitialized");
    }
    else
    {
        /* Draw the property */
        const type* property_type = property->get_type();
        bool read_only = _view->has_input_connected();

        if( property_type->is<i32_t>() )
        {
            auto integer = (i32_t)*property->value();

            if (ImGui::InputInt(label.c_str(), &integer, 0, 0, input_text_flags ) && !read_only )
            {
                property->set(integer);
                changed |= true;
            }
        }
        else if( property_type->is<double>() )
        {
            auto d = (double)*property->value();

            if (ImGui::InputDouble(label.c_str(), &d, 0.0F, 0.0F, "%g", input_text_flags ) && !read_only )
            {
                property->set(d);
                changed |= true;
            }
        }
        else if( property_type->is<std::string>() )
        {
            char str[255];
            snprintf(str, 255, "%s", (const char*)*property->value() );

            if ( ImGui::InputText(label.c_str(), str, 255, input_text_flags ) && !read_only )
            {
                property->set( std::string(str) );
                changed |= true;
            }
        }
        else if( property_type->is<bool>() )
        {
            auto b = (bool)*property->value();

            if (ImGui::Checkbox(label.c_str(), &b) && !read_only )
            {
                property->set(b);
                changed |= true;
            }
        }
        else
        {
            auto property_as_string = (*property )->to<std::string>();
            ImGui::Text( "%s", property_as_string.c_str());
        }
    }

    return changed;
}

bool NodeView::is_inside(NodeView* _nodeView, Rect _rect, Space _space)
{
	return Rect::contains(_rect, _nodeView->rect(_space) );
}

void NodeView::draw_as_properties_panel(NodeView *_view, bool *_show_advanced)
{
    Node* node = _view->m_owner.get();
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](PropertyView* _property_view)
    {
        Property*property = _property_view->get_property();
        // label (<name> (<type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s): ",
                property->get_name().c_str(),
                property->get_type()->get_name());

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGuiEx::BeginTooltip() )
        {
            const auto variant = property->value();
            ImGui::Text("initialized: %s,\n"
                        "defined:     %s,\n"
                        "Source token:\n"
                        "%s\n",
                        variant->is_initialized() ? "true" : "false",
                        variant->is_defined()     ? "true" : "false",
                         property->token.json().c_str()
                        );
            ImGuiEx::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool edited = NodeView::draw_property_view(_property_view, nullptr);
        node->dirty |= edited;

    };

    ImGui::Text("Name:       \"%s\"" , node->name.c_str());
    ImGui::Text("Class:      %s"     , node->get_type()->get_name());

    // Draw exposed input properties
    ImGui::Separator();
    ImGui::Text("Input(s):" );
    ImGui::Separator();
    ImGui::Indent();
    if( _view->m_property_views_with_input_only.empty() )
    {
        ImGui::Text("None.");
        ImGui::Separator();
    }
    else
    {
        for (auto& property_view : _view->m_property_views_with_input_only )
        {
            draw_labeled_property_view( property_view );
            ImGui::Separator();
        }
    }
    ImGui::Unindent();

    // Draw exposed output properties
    ImGui::Text("Output(s):" );
    ImGui::Separator();
    ImGui::Indent();
    if( _view->m_property_views_with_output_or_inout.empty() )
    {
        ImGui::Text("None.");
        ImGui::Separator();
    }
    else
    {
        for (auto& each_property_view: _view->m_property_views_with_output_or_inout )
        {
            draw_labeled_property_view( each_property_view );
            ImGui::Separator();
        }
    }
    ImGui::Unindent();

    if ( fw::g_conf->debug )
    {
        ImGui::Text("Debug info:" );
        // Draw exposed output properties
        if( ImGui::TreeNode("Other Properties") )
        {
            draw_labeled_property_view( _view->m_property_view_this );
            ImGui::TreePop();
        }

        // Components
        if( ImGui::TreeNode("Components") )
        {
            for (PoolID<const Component> component : node->get_components() )
            {
                ImGui::BulletText("%s", component->get_type()->get_name());
            }
            ImGui::TreePop();
        }

        if( ImGui::TreeNode("Slots") )
        {
            auto draw_node_list = [](const char *label, const std::vector<PoolID<Node>> _nodes )
            {
                if( !ImGui::TreeNode(label) )
                {
                    return;
                }

                if ( _nodes.empty() )
                {
                    ImGui::BulletText( "None" );
                }

                for (const PoolID<Node>& each_node : _nodes )
                {
                    ImGui::BulletText("- %s", each_node->name.c_str());
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
            Physics* physics_component = node->get_component<Physics>().get();
            ImGui::Checkbox("On/Off", &physics_component->is_active);
            int i = 0;
            for(NodeViewConstraint& constraint : physics_component->constraints)
            {
                constraint.draw_view();
            }
            ImGui::TreePop();
        }

        // Scope specific:
        if (Scope* scope = node->get_component<Scope>().get() )
        {
            if( ImGui::TreeNode("Variables") )
            {
                auto vars = scope->variables();
                for (auto eachVar : vars)
                {
                    ImGui::BulletText("%s: %s", eachVar->name.c_str(), eachVar->property()->to<std::string>().c_str());
                }
                ImGui::TreePop();
            }
        }

        if( ImGui::TreeNode("Misc:") )
        {
            // dirty state
            ImGui::Separator();
            bool b = node->dirty;
            ImGui::Checkbox("Is dirty ?", &b);

            // Parent graph
            {
                std::string parentName = "NULL";

                if (Graph* parent_graph = node->parent_graph)
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

                if (Node* parent = node->find_parent().get())
                {
                    parentName = parent->name + (parent->dirty ? " (dirty)" : "");
                }
                ImGui::Text("Parent node is \"%s\"", parentName.c_str());
            }
            ImGui::TreePop();
        }
    }
    ImGui::Separator();
}

void NodeView::constraint_to_rect(NodeView* _view, Rect _rect)
{
	
	if ( !NodeView::is_inside(_view, _rect, WORLD_SPACE )) {

        _rect.expand( Vec2( -2, -2 ) ); // shrink

		auto nodeRect = _view->rect( WORLD_SPACE );

		auto left  = _rect.min.x - nodeRect.min.x;
		auto right = _rect.max.x - nodeRect.max.x;
		auto up    = _rect.min.y - nodeRect.min.y;
		auto down  = _rect.max.y - nodeRect.max.y;

		     if ( left > 0 )  nodeRect.translate_x( left );
		else if ( right < 0 ) nodeRect.translate_x( right );
			 
			 if ( up > 0 )  nodeRect.translate_y( up );
		else if ( down < 0 )nodeRect.translate_y( down );

        _view->position( nodeRect.center(), PARENT_SPACE );
	}

}

void NodeView::set_view_detail(NodeViewDetail _viewDetail)
{
    NodeView::s_view_detail = _viewDetail;

    for( auto& eachView : Pool::get_pool()->get_all<NodeView>())
    {
        for( auto& property_view : eachView.m_property_views )
        {
            property_view.reset();
        }
    }
}

Rect NodeView::get_rect(Space space, NodeViewFlags flags) const
{
    const bool recursively   = flags & NodeViewFlag_RECURSIVELY;
    const bool ignore_self   = flags & NodeViewFlag_IGNORE_SELF;
    const bool ignore_pinned = flags & NodeViewFlag_IGNORE_PINNED;

    if( !recursively )
    {
        return View::rect(space);
    }

    std::vector<Rect> rects;

    if ( !ignore_self && is_visible )
    {
        rects.push_back( View::rect(space) );
    }

    auto push_view_rect = [&](PoolID<NodeView> view_id)
    {
        NodeView* view = view_id.get();
        if( !view) return;
        if( !view->is_visible ) return;
        if( view->m_pinned && ignore_pinned ) return;
        if( view->m_owner->should_be_constrain_to_follow_output( this->m_owner ) )
        {
            Rect rect = view->get_rect(space, flags);
            rects.push_back( rect );
        }
    };

    auto children = get_adjacent(SlotFlag_CHILD);
    std::for_each( children.begin(), children.end(), push_view_rect );

    auto inputs   = get_adjacent(SlotFlag_INPUT);
    std::for_each( inputs.begin()  , inputs.end()  , push_view_rect );

    Rect result = Rect::bbox(rects);

#ifdef NDBL_DEBUG
    Rect screen_rect = result;
    screen_rect.translate( position(space) - position(PARENT_SPACE) );
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
    std::vector<Rect> rects;

    for (auto eachView : _views)
    {
        Rect rect = eachView->get_rect(space, flags);
        rects.push_back( rect );
    }

    return Rect::bbox( rects );
}

std::vector<Rect> NodeView::get_rects(const std::vector<NodeView*>& _in_views, Space space, NodeViewFlags flags)
{
    std::vector<Rect> rects;
    for (auto each_target : _in_views )
    {
        rects.push_back( each_target->get_rect(space, flags ));
    }
    return std::move( rects );
}

void NodeView::set_expanded_rec(bool _expanded)
{
    set_expanded(_expanded);
    for(PoolID<NodeView> each_child_view : get_adjacent(SlotFlag_CHILD) )
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
        if( _visible || has_not_output || each_child_view->m_owner->should_be_constrain_to_follow_output( m_owner ) )
        {
            if ( _recursive && each_child_view->m_expanded) // propagate only if expanded
            {
                each_child_view->set_children_visible(_visible, true);
                each_child_view->set_inputs_visible(_visible, true);
            }
            each_child_view->is_visible = _visible;
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

    if( _view->is_visible )
    {
        return _view;
    }

    Node* parent = _view->m_owner->find_parent().get();
    if ( !parent )
    {
        return _view;
    }

    NodeView* parent_view = parent->get_component<NodeView>().get();
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


bool NodeView::is_any_selected()
{
    return NodeView::get_selected().get() != nullptr;
}

bool NodeView::is_dragged() const
{
    return s_dragged == m_id;
}

Vec2 NodeView::get_slot_normal( const Slot& slot ) const
{
    // Alignment is usually not a corner, so we don't need to normalize.
    return m_slot_views[(u8_t)slot.id].alignment();
}

Vec2 NodeView::get_slot_pos( const Slot& slot )
{
    // TODO: use 3x3 matrices to simplify code

    if( slot.type() == SlotFlag_TYPE_VALUE && slot.get_property()->is_this() )
    {
        Rect r = rect( WORLD_SPACE );
        return r.center() + r.size() * m_slot_views[(u8_t)slot.id].alignment();
    }
    Rect property_rect = m_property_views.at( (u32_t)slot.property ).screen_rect;
    return property_rect.center()
         + property_rect.size() * m_slot_views[(u8_t)slot.id].alignment();
}

Rect NodeView::get_slot_rect( const Slot& _slot, i8_t _count ) const
{
     return get_slot_rect( m_slot_views[_slot.id.m_value], _count );
}

Rect NodeView::get_slot_rect( const SlotView& _slot_view, i8_t _pos ) const
{
    Rect result({0.0f, 0.0f }, g_conf->ui_slot_size );
    result.translate_y( -result.size().y * 0.5f ); // Center vertically
    result.translate_x( g_conf->ui_slot_size.x * float( _pos )      // x offset
                      + g_conf->ui_slot_gap * float( 1 + _pos ) ); // x gap
    result.translate_y( _slot_view.alignment().y * result.size().y ); // align top/bottom
    Rect view_rect = rect( WORLD_SPACE );
    result.translate( _slot_view.alignment() * view_rect.size() + view_rect.center() ); // align slot with nodeview

    return result;
}

std::vector<PoolID<NodeView>> NodeView::get_adjacent(SlotFlags flags) const
{
    return GraphUtil::adjacent_components<NodeView>(m_owner.get(), flags);
}

void NodeView::set_color( const Vec4* _color, ColorType _type )
{
    m_colors[_type] = _color;
}

Vec4 NodeView::get_color( ColorType _type ) const
{
    return *m_colors[_type];
}

bool NodeView::none_is_visible( std::vector<NodeView*> _views )
{
    auto is_visible = [](const NodeView* view) { return view->is_visible; };
    return std::find_if(_views.begin(), _views.end(), is_visible) == _views.end();
}
