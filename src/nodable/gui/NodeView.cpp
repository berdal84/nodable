#include "NodeView.h"

#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeViewConstraint.h"
#include "Physics.h"
#include "PropertyView.h"
#include "SlotView.h"
#include "core/Graph.h"
#include "core/GraphUtil.h"
#include "core/InvokableComponent.h"
#include "core/LiteralNode.h"
#include "core/Pool.h"
#include "core/Scope.h"
#include "core/VariableNode.h"
#include "core/language/Nodlang.h"
#include "fw/core/math.h"
#include "fw/core/reflection/registration.h"
#include <algorithm> // for std::max
#include <cmath> // for sinus
#include <vector>

constexpr ImVec2 NODE_VIEW_DEFAULT_SIZE(10.0f, 35.0f);

using namespace ndbl;
using namespace fw;

REGISTER
{
    fw::registration::push_class<NodeView>("NodeView")
        .extends<Component>()
        .extends<fw::View>();
}

PoolID<NodeView>   NodeView::s_selected;
PoolID<NodeView>   NodeView::s_dragged;

// TODO: move those values into the configuration
NodeViewDetail     NodeView::s_view_detail                       = NodeViewDetail::Default;
const float        NodeView::s_property_input_size_min           = 10.0f;
const ImVec2 NodeView::s_property_input_toggle_button_size(10.0, 25.0f);

NodeView::NodeView()
        : Component()
        , fw::View()
        , m_position(500.0f, -1.0f)
        , m_size(NODE_VIEW_DEFAULT_SIZE)
        , m_opacity(1.0f)
        , m_expanded(true)
        , m_pinned(false)
        , m_border_color_selected(1.0f, 1.0f, 1.0f)
        , m_property_view_this(nullptr)
        , m_edition_enable(true)
        , m_property_views()
{
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
        else if ( !m_owner->find_slot( each_prop.id, SlotFlag_OUTPUT ) )
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
    for(Slot& slot : m_owner->slots.data() )
    {
        ImVec2 alignment;

        switch ( slot.flags )
        {
            case SlotFlag_INPUT:  alignment.y = -0.5f; break;
            case SlotFlag_PREV:   alignment   = { -0.5f, -0.5f}; break;
            case SlotFlag_OUTPUT: alignment   = slot.get_property()->is_this() ? ImVec2{-0.5f, 0.0f}
                                                                               : ImVec2{ 0.0f, 0.5f}; break;
            case SlotFlag_NEXT:   alignment   = { -0.5f, 0.5f}; break;
            case SlotFlag_CHILD:
            case SlotFlag_PARENT: break; // won't be displayed, let's keep default alignment (0,0)
            default:
                FW_EXPECT(false, "unhandled slot flags")
        }

        m_slot_views.emplace_back( SlotView{slot, alignment} );
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
    if ( auto* variable = fw::cast<const VariableNode>(_node) )
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
    if( s_selected == new_selection ) return;

    // Handle de-selection
    if( s_selected )
    {
        Event event{ EventType_node_view_deselected };
        event.node.view = s_selected;
        fw::EventManager::get_instance().push_event((fw::Event&)event);
        s_selected.reset();
    }

    // Handle selection
    if( new_selection )
    {
        Event event{ EventType_node_view_selected };
        event.node.view = new_selection;
        fw::EventManager::get_instance().push_event((fw::Event&)event);
        s_selected = new_selection;
    }
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
    return &m_property_views.at(_id);
}

void NodeView::set_position(ImVec2 _position, fw::Space origin)
{
    switch (origin)
    {
        case fw::Space_Local: m_position = _position; break;
        case fw::Space_Screen: m_position = _position - m_screen_space_content_region.GetTL(); break;
        default:
            FW_EXPECT(false, "OriginRef_ case not handled, cannot compute perform set_position(...)")
    }
}

ImVec2 NodeView::get_position(fw::Space origin, bool round) const
{
    // compute position depending on space
    ImVec2 result = m_position;
    if (origin == fw::Space_Screen) result += m_screen_space_content_region.GetTL();

    // return rounded or not if needed
    if(round) return fw::math::round(result);
    return result;
}

void NodeView::translate(ImVec2 _delta, bool _recurse)
{
    ImVec2 current_local_position = get_position(fw::Space_Local);
    set_position( current_local_position + _delta, fw::Space_Local);

	if ( !_recurse ) return;

    for(auto each_input_view : get_adjacent(SlotFlag_INPUT)  )
    {
        if ( each_input_view && !each_input_view->m_pinned && each_input_view->should_follow_output( this->m_id ) )
        {
            each_input_view->translate(_delta, true);
        }
    }
}

void NodeView::arrange_recursively(bool _smoothly)
{
    for (auto each_input: get_adjacent(SlotFlag_INPUT) )
    {
        if ( !each_input->m_pinned && each_input->should_follow_output( this->m_id ))
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
    if(m_opacity != 1.0f) fw::math::lerp(m_opacity, 1.0f, 10.0f * _deltaTime);
	return true;
}

bool NodeView::draw()
{
	bool        changed   = false;
    Node*       node      = m_owner.get();
	Config&     config    = Nodable::get_instance().config;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    FW_ASSERT(node != nullptr);

    // Draw Node slots (in background)
    bool is_slot_hovered = false;
    {
        ImColor color          = config.ui_node_nodeslotColor;
        ImColor border_color   = config.ui_node_borderColor;
        ImColor hover_color    = config.ui_node_nodeslotHoveredColor;
        ImRect  node_view_rect = get_screen_rect();

        std::unordered_map<SlotFlags, int> count_by_flags{{SlotFlag_NEXT, 0}, {SlotFlag_PREV, 0}};
        for ( SlotView& slot_view : m_slot_views )
        {
            if( slot_view.slot().type() == SlotFlag_TYPE_CODEFLOW )
            {
                int& count = count_by_flags[slot_view.slot().flags];
                ImRect rect = get_slot_rect( slot_view, config, count );
                SlotView::draw_slot_rectangle( draw_list, slot_view, rect, color, border_color, hover_color, m_edition_enable);
                is_slot_hovered |= ImGui::IsItemHovered();
                count++;
            }
        }
    }

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
	const auto halfSize = m_size / 2.0;
    ImVec2 node_screen_center_pos = get_position(fw::Space_Screen, true);
    const ImVec2 &node_top_left_corner = node_screen_center_pos - halfSize;
    ImGui::SetCursorScreenPos(node_top_left_corner); // start from th top left corner
	ImGui::PushID(this);


	// Draw the background of the Group
    auto border_color = is_selected(m_id) ? m_border_color_selected : get_color(Color_BORDER);
    DrawNodeRect(
            node_top_left_corner, node_top_left_corner + m_size,
            get_color(Color_FILL), get_color(Color_BORDER_HIGHLIGHT), get_color(Color_SHADOW), border_color,
            is_selected(m_id), 5.0f, config.ui_node_padding);

    // Add an invisible just on top of the background to detect mouse hovering
	ImGui::SetCursorScreenPos(node_top_left_corner);
	ImGui::InvisibleButton("node", m_size);
    ImGui::SetItemAllowOverlap();
    ImGui::SetCursorScreenPos(node_top_left_corner + config.ui_node_padding); // top left corner + padding in x and y.
	ImGui::SetCursorPosX( ImGui::GetCursorPosX() + config.ui_node_propertyslotRadius); // add + space for "this" left slot
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
        fw::ImGuiEx::ShadowedText(ImVec2(1.0f), get_color(Color_BORDER_HIGHLIGHT), label.c_str()); // text with a lighter shadow (encrust effect)

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

        ImGui::SetCursorPosX( ImGui::GetCursorPosX() + config.ui_node_padding * 2.0f);
        ImGui::SetCursorPosY( ImGui::GetCursorPosY() + config.ui_node_padding );
    ImGui::EndGroup();

    // Ends the Window
    //----------------
    ImVec2 node_top_right_corner = ImGui::GetCursorScreenPos();
    m_size.x = std::max( 1.0f, std::ceil(ImGui::GetItemRectSize().x));
    m_size.y = std::max( 1.0f, std::ceil(node_top_right_corner.y - node_top_left_corner.y ));

    // Draw Property in/out slots
    {
        float radius      = config.ui_node_propertyslotRadius;
        ImColor color     = config.ui_node_nodeslotColor;
        ImColor borderCol = config.ui_node_borderColor;
        ImColor hoverCol  = config.ui_node_nodeslotHoveredColor;

        for( auto& slot_view: m_slot_views )
        {
            if( slot_view.slot().type() == SlotFlag_TYPE_VALUE )
            {
                ImVec2 screen_pos = get_slot_pos(slot_view.slot());
                SlotView::draw_slot_circle( draw_list, slot_view, screen_pos, radius, color, borderCol, hoverCol, m_edition_enable );
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

    m_is_hovered = is_node_hovered || is_slot_hovered;

	return changed;
}
void NodeView::DrawNodeRect(ImVec2 rect_min, ImVec2 rect_max, ImColor color, ImColor border_highlight_col, ImColor shadow_col, ImColor border_col, bool selected, float border_radius, float padding)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw the rectangle under everything
    fw::ImGuiEx::DrawRectShadow(rect_min, rect_max, border_radius, 4, ImVec2(1.0f), shadow_col);
    draw_list->AddRectFilled(rect_min, rect_max, color, border_radius);
    draw_list->AddRect(rect_min + ImVec2(1.0f), rect_max, border_highlight_col, border_radius);
    draw_list->AddRect(rect_min, rect_max, border_col, border_radius);

    // Draw an additional blinking rectangle when selected
    if (selected)
    {
        auto alpha   = sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
        float offset = 4.0f;
        draw_list->AddRect(rect_min - ImVec2(offset), rect_max + ImVec2(offset), ImColor(1.0f, 1.0f, 1.0f, float(alpha) ), border_radius + offset, ~0, offset / 2.0f);
    }

}

bool NodeView::_draw_property_view(PropertyView* _view)
{
    bool            changed            = false;
    Property*       property           = _view->get_property();
    bool            is_defined         = property->value()->is_defined();
    const fw::type* owner_type         = m_owner->get_type();
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

    if ( fw::ImGuiEx::BeginTooltip() )
    {
        ImGui::Text("%s %s\n", property->get_type()->get_name(), property->get_name().c_str());

        std::string  source_code;
        if( property->get_type()->is<PoolID<Node>>() || m_owner->find_slot( property->id, SlotFlag_OUTPUT ))
        {
            source_code = Nodlang::get_instance().serialize_node( source_code, m_owner );
        }
        else
        {
            source_code = Nodlang::get_instance().serialize_property(source_code, property );
        }

        ImGui::Text("source: \"%s\"", source_code.c_str());

        fw::ImGuiEx::EndTooltip();
    }

    // memorize property view rect (screen space)
    // enlarge rect to fit node_view top/bottom
    _view->screen_rect = {
        ImVec2{ImGui::GetItemRectMin().x, get_screen_rect().Min.y} ,
        ImVec2{ImGui::GetItemRectMax().x, get_screen_rect().Max.y}
    };
    fw::ImGuiEx::DebugCircle( _view->screen_rect.GetCenter(), 2.5f, ImColor(0,0,0));

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

        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4) variable->get_component<NodeView>()->get_color(Color_FILL) );
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
        const fw::type* property_type = property->get_type();
        bool has_input_connected      = _view->has_input_connected();

        if( property_type->is<i32_t>() )
        {
            auto integer = (i32_t)*property->value();

            if (ImGui::InputInt(label.c_str(), &integer, 0, 0, input_text_flags ) && !has_input_connected )
            {
                property->set(integer);
                changed |= true;
            }
        }
        else if( property_type->is<double>() )
        {
            auto d = (double)*property->value();

            if (ImGui::InputDouble(label.c_str(), &d, 0.0F, 0.0F, "%g", input_text_flags ) && !has_input_connected )
            {
                property->set(d);
                changed |= true;
            }
        }
        else if( property_type->is<std::string>() )
        {
            char str[255];
            snprintf(str, 255, "%s", (const char*)*property->value() );

            if ( ImGui::InputText(label.c_str(), str, 255, input_text_flags ) && !has_input_connected )
            {
                property->set( std::string(str) );
                changed |= true;
            }
        }
        else if( property_type->is<bool>() )
        {
            std::string checkBoxLabel = property->get_name();

            auto b = (bool)*property->value();

            if (ImGui::Checkbox(label.c_str(), &b ) && !has_input_connected )
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

bool NodeView::is_inside(NodeView* _nodeView, ImRect _rect)
{
	return _rect.Contains(_nodeView->get_rect());
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
        if ( fw::ImGuiEx::BeginTooltip() )
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
            fw::ImGuiEx::EndTooltip();
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

    if ( ImGui::TreeNode("Debug") )
    {
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
        ImGui::Separator();
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
        ImGui::Separator();
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

                if (Node* parent = node->get_parent().get())
                {
                    parentName = parent->name + (parent->dirty ? " (dirty)" : "");
                }
                ImGui::Text("Parent node is \"%s\"", parentName.c_str());
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
}

void NodeView::constraint_to_rect(NodeView* _view, ImRect _rect)
{
	
	if ( !NodeView::is_inside(_view, _rect)) {

		_rect.Expand(ImVec2(-2, -2)); // shrink

		auto nodeRect = _view->get_rect();

		auto newPos = _view->get_position(fw::Space_Local, true);

		auto left  = _rect.Min.x - nodeRect.Min.x;
		auto right = _rect.Max.x - nodeRect.Max.x;
		auto up    = _rect.Min.y - nodeRect.Min.y;
		auto down  = _rect.Max.y - nodeRect.Max.y;

		     if ( left > 0 )  nodeRect.TranslateX(left);
		else if ( right < 0 ) nodeRect.TranslateX(right);
			 
			 if ( up > 0 )    nodeRect.TranslateY(up);
		else if ( down < 0 )  nodeRect.TranslateY(down);

        _view->set_position(nodeRect.GetCenter(), fw::Space_Local);
	}

}

bool NodeView::is_exposed( ID<Property> _id )const
{
    FW_EXPECT(false, "TODO: implement (Take in consideration that now any property as a view)");
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

ImRect NodeView::get_rect(bool _recursively, bool _ignorePinned, bool _ignoreMultiConstrained, bool _ignoreSelf) const
{
    if( !_recursively)
    {
        ImVec2 local_position = get_position(fw::Space_Local);
        ImRect rect{local_position, local_position};
        rect.Expand(m_size);

        return rect;
    }

    ImRect result_rect( ImVec2(std::numeric_limits<float>::max()), ImVec2(-std::numeric_limits<float>::max()) );

    if ( !_ignoreSelf && m_is_visible )
    {
        ImRect self_rect = get_rect(false);
        fw::ImGuiEx::EnlargeToInclude(result_rect, self_rect);
    }

    auto enlarge_to_fit_all = [&](PoolID<NodeView> view_id)
    {
        NodeView* view = view_id.get();
        if( !view) return;

        if ( view->m_is_visible && !(view->m_pinned && _ignorePinned) && view->should_follow_output( this->m_id ) )
        {
            ImRect child_rect = view->get_rect(true, _ignorePinned, _ignoreMultiConstrained);
            fw::ImGuiEx::EnlargeToInclude(result_rect, child_rect);
        }
    };

    auto children = get_adjacent(SlotFlag_CHILD);
    std::for_each( children.begin(), children.end(), enlarge_to_fit_all);

    auto inputs   = get_adjacent(SlotFlag_INPUT);
    std::for_each( inputs.begin()  , inputs.end()  , enlarge_to_fit_all);

    fw::ImGuiEx::DebugRect(result_rect.Min, result_rect.Max, IM_COL32( 0, 255, 0, 255 ),4 );

    return result_rect;
}

void NodeView::translate_to(ImVec2 desiredPos, float _factor, bool _recurse)
{
    ImVec2 delta(desiredPos - m_position);

    bool isDeltaTooSmall = delta.x * delta.x + delta.y * delta.y < 0.01f;
    if (!isDeltaTooSmall)
    {
        auto factor = std::min(1.0f, _factor);
        translate(delta * factor, _recurse);
    }
}

ImRect NodeView::get_rect(
        const std::vector<NodeView *> &_views,
        bool _recursive,
        bool _ignorePinned,
        bool _ignoreMultiConstrained)
{
    ImRect rect(ImVec2(std::numeric_limits<float>::max()), ImVec2(-std::numeric_limits<float>::max()) );

    for (auto eachView : _views)
    {
        if ( eachView->m_is_visible )
        {
            auto each_rect = eachView->get_rect(_recursive, _ignorePinned, _ignoreMultiConstrained);
            fw::ImGuiEx::EnlargeToInclude(rect, each_rect);
        }
    }
    return rect;
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

bool NodeView::should_follow_output(PoolID<const NodeView> output) const
{
    auto outputs = get_adjacent(SlotFlag_OUTPUT);
    return outputs.empty() || outputs[0] == output;
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
        if( _visible || has_not_output || each_child_view->should_follow_output( m_id ) )
        {
            if ( _recursive && each_child_view->m_expanded) // propagate only if expanded
            {
                each_child_view->set_children_visible(_visible, true);
                each_child_view->set_inputs_visible(_visible, true);
            }
            each_child_view->set_visible(_visible);
        }
    }
}

void NodeView::expand_toggle()
{
    set_expanded(!m_expanded);
}

NodeView* NodeView::substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive)
{
    if( _view->is_visible() )
    {
        return _view;
    }

    Node* parent = _view->m_owner->get_parent().get();
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

void NodeView::expand_toggle_rec()
{
    return set_expanded_rec(!m_expanded);
}

ImRect NodeView::get_screen_rect() const
{
    ImVec2 half_size = m_size / 2.0f;
    ImVec2 screen_pos = get_position(fw::Space_Screen, false);
    return {screen_pos - half_size, screen_pos + half_size};
}

bool NodeView::is_any_selected()
{
    return NodeView::get_selected().get() != nullptr;
}

bool NodeView::is_dragged() const
{
    return s_dragged == m_id;
}

ImVec2 NodeView::get_slot_pos( const Slot& slot )
{
    // TODO: use 3x3 matrices to simplify code

    if( slot.type() == SlotFlag_TYPE_VALUE && slot.property == THIS_PROPERTY_ID )
    {
        return get_screen_rect().GetCenter()
             + get_screen_rect().GetSize() * m_slot_views[slot.id].alignment();
    }
    ImRect property_rect = m_property_views.at( slot.property ).screen_rect;
    return property_rect.GetCenter()
         + property_rect.GetSize() * m_slot_views[slot.id].alignment();
}

ImRect NodeView::get_slot_rect( SlotView& _slot_view, const Config& _config, i8_t _count ) const
{
    // pick a corner
    ImRect view_rect = get_screen_rect();
    ImVec2 left_corner  = _slot_view.alignment() * view_rect.GetSize() // relative position
                        + view_rect.GetCenter(); // left aligned

    // compute slot size
    ImVec2 size(std::min(_config.ui_node_slot_width,  get_size().x), std::min(_config.ui_node_slot_height, get_size().y));
    ImRect rect(left_corner, left_corner + size);

    rect.Translate( ImVec2(size.x * _count, -rect.GetSize().y * 0.5f) );
    rect.Expand( ImVec2(- _config.ui_node_slot_padding, 0.0f));

    return rect;
}

std::vector<PoolID<NodeView>> NodeView::get_adjacent(SlotFlags flags) const
{
    return GraphUtil::adjacent_components<NodeView>(m_owner.get(), flags);
}
