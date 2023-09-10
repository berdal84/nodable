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
        , pinned(false)
        , m_border_color_selected(1.0f, 1.0f, 1.0f)
        , m_exposed_this_property_view(nullptr)
        , m_edition_enable(true)
{
}

NodeView::~NodeView()
{
    // delete PropertyViews
    for ( auto& [_, property] : m_exposed_properties ) delete property;

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

void NodeView::expose(Property* _property)
{
    FW_EXPECT(false, "TODO: is this function still necessary since we have Slots?")
//    PropertyView* property_view = new PropertyView(_property, m_id);
//    if (_property == m_owner->as_prop() )
//    {
//        property_view->output()->m_display_side = SlotView::Side::Left; // force to be displayed on the left
//        m_exposed_this_property_view = property_view;
//    }
//    else
//    {
//        if (_property->get_allowed_connection() == Way::In)
//        {
//            m_exposed_input_only_properties.push_back(property_view);
//        }
//        else
//        {
//            m_exposed_out_or_inout_properties.push_back(property_view);
//        }
//    }
//
//    m_exposed_properties.insert({_property, property_view});
}

void NodeView::set_owner(PoolID<Node> node)
{
    Component::set_owner(node);

    if( node == ID_NULL )
    {
        return;
    }

    const Config& config = Nodable::get_instance().config;
    std::vector<Property*> not_exposed;

    // 1. Expose properties (make visible)
    //------------------------------------

    //  We expose first the properties which allows input connections

    for ( Property& property : node->props )
    {
        if ( property.get_visibility() == Visibility::Always && property.allows_connection(Way::In) )
        {
            expose(&property);
        }
        else
        {
            not_exposed.push_back(&property);
        }
    }

    // Then we expose node which allows output connection (if they are not yet exposed)
    for (auto property_id : not_exposed)
    {
        Property* property = property_id;
        if (property->get_visibility() == Visibility::Always && property->allows_connection(Way::Out))
        {
            expose(property);
        }
    }

    if ( Property* this_property = node->get_prop(THIS_PROPERTY) )
    {
        expose(this_property);
    }

    // 2. SlotViews
    //--------------

    FW_EXPECT(false, "Create")

    // 3. Listen to connection/disconnections
    //---------------------------------------

    PoolID<NodeView> id = m_id;
    auto synchronize_view = [id, node](SlotBag::Event event)
    {
        FW_EXPECT(false, "TODO: update children, predecessors, inputs, and outputs cache vector<PoolID<NodeView>>")
        id->children = GraphUtil::adjacent_components<NodeView>(node.get(), Relation::CHILD_PARENT, Way::In);
    };
    node->on_slot_change.connect(synchronize_view);

    // update label
    update_labels_from_name(node.get());
    node->on_name_change.connect([id](PoolID<Node> _node) {
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
        fw::EventManager::get_instance().push_event((fw::Event&)event);
        event.node.view = new_selection;
        s_selected = new_selection;
    }
}

PoolID<NodeView> NodeView::get_selected()
{
	return s_selected;
}

void NodeView::start_drag(PoolID<NodeView> _view)
{
	if( !is_any_dragged() && SlotView::get_dragged() ) // Prevent dragging node while dragging slot
    {
        s_dragged = _view;
    }
}

bool NodeView::is_any_dragged()
{
	return get_dragged().get() != nullptr;
}

PoolID<NodeView> NodeView::get_dragged()
{
	return s_dragged;
}

bool NodeView::is_selected(PoolID<NodeView> view)
{
	return s_selected == view;
}

const PropertyView* NodeView::get_property_view(const Property * _property)const
{
    auto found = m_exposed_properties.find(_property);
    if( found == m_exposed_properties.end() ) return nullptr;
    return found->second;
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

    for(auto eachInput : m_owner->inputs )
    {
        auto eachInputView = eachInput->get_component<NodeView>();
        if ( eachInputView && !eachInputView->pinned && eachInputView->should_follow_output( this->m_id ) )
        {
            eachInputView->translate(_delta, true);
        }
    }
}

void NodeView::arrange_recursively(bool _smoothly)
{
    for (auto each_input_view: inputs.content() )
    {
        if (each_input_view->should_follow_output( this->m_id ))
        {
            each_input_view->arrange_recursively();
        }
    }

    for (auto each_child_view: children.content() )
    {
        each_child_view->arrange_recursively();
    }

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        update(float(1000));
    }

    pinned = false;
}

bool NodeView::update(float _deltaTime)
{
    if(m_opacity != 1.0f) fw::math::lerp(m_opacity, 1.0f, 10.0f * _deltaTime);
	return true;
}

bool NodeView::draw()
{
	bool      changed  = false;
    Node*     node     = m_owner.get();
	Config&   config   = Nodable::get_instance().config;

    FW_ASSERT(node != nullptr);

    // Draw Node slots (in background)
    bool is_slot_hovered = false;
    {
        ImColor color        = config.ui_node_nodeslotColor;
        ImColor hoveredColor = config.ui_node_nodeslotHoveredColor;

        auto draw_and_handle_evt = [&](Nodeslot* slot)
        {
            Nodeslot::draw(slot, color, hoveredColor, m_edition_enable);
            is_slot_hovered |= ImGui::IsItemHovered();
        };

        std::for_each(m_predecessors.begin(), m_predecessors.end(), draw_and_handle_evt);
        std::for_each(m_successors.begin()  , m_successors.end()  , draw_and_handle_evt);

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
        fw::ImGuiEx::ShadowedText(ImVec2(1.0f), get_color(Color_BORDER_HIGHLIGHT), label.c_str()); // text with a lighter shadow (incrust effect)

        ImGui::SameLine();

        ImGui::BeginGroup();

        // draw properties
        auto draw_property_lambda = [&](PropertyView* view) {
            ImGui::SameLine();
            changed |= draw_property_view( view );
        };
        std::for_each(m_exposed_input_only_properties.begin(), m_exposed_input_only_properties.end(), draw_property_lambda);
        std::for_each(m_exposed_out_or_inout_properties.begin(), m_exposed_out_or_inout_properties.end(), draw_property_lambda);


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

        if ( m_exposed_this_property_view )
        {
            SlotView::draw_slot_circle(m_exposed_this_property_view->output(), radius, color, borderCol, hoverCol, m_edition_enable);
            is_slot_hovered |= ImGui::IsItemHovered();
        }

        for( auto& propertyView : m_exposed_input_only_properties )
        {
            SlotView::draw_slot_circle(propertyView->input(), radius, color, borderCol, hoverCol, m_edition_enable);
            is_slot_hovered |= ImGui::IsItemHovered();
        }

        for( auto& propertyView : m_exposed_out_or_inout_properties )
        {
            if ( propertyView->input() )
            {
                SlotView::draw_slot_circle(propertyView->input(), radius, color, borderCol, hoverCol, m_edition_enable);
                is_slot_hovered |= ImGui::IsItemHovered();
            }

            if ( propertyView->output() )
            {
                SlotView::draw_slot_circle(propertyView->output(), radius, color, borderCol, hoverCol, m_edition_enable);
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

        ImGui::MenuItem("Pinned", "", &pinned, true);

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
        set_selected( this->m_id );
    }

	// Mouse dragging
	if (get_dragged() != id())
	{
		if( get_dragged().get() == nullptr && ImGui::IsMouseDown(0) && is_node_hovered && ImGui::IsMouseDragPastThreshold(0))
        {
            start_drag( id() );
        }
	}
	else if ( ImGui::IsMouseReleased(0))
	{
        start_drag({});
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

bool NodeView::draw_property_view(PropertyView* _view)
{
    bool      changed      = false;
    Property* property     = _view->property();
    bool      is_defined   = property->value()->is_defined();
    const fw::type* owner_type = property->owner()->get_type();

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
        _view->show_input |= property->is_connected_to_variable();
        // Shows variable property only if they are not connected (don't need to show anything, the variable name is already displayed on the node itself)
        _view->show_input |= is_defined && (owner_type->is<VariableNode>() || !property->has_input_connected());
    }

    // input
    float input_size = NodeView::s_property_input_toggle_button_size.x;

    if ( _view->show_input )
    {
        bool limit_size = !property->get_type()->is<bool>();

        if ( limit_size )
        {
            // try to draw an as small as possible input field
            std::string str;
            if (property->is_connected_to_variable())
            {
                str = property->get_connected_variable()->name;
            }
            else
            {
                str = property->to<std::string>();
            }
            input_size = 5.0f + std::max(ImGui::CalcTextSize(str.c_str()).x, NodeView::s_property_input_size_min);
            ImGui::PushItemWidth(input_size);
        }
        changed = NodeView::draw_property(property, nullptr);

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

        // Generate the property's source code
        std::string source_code;
        if( property->get_type()->is<PoolID<Node>>() || !property->allows_connection(Way::In)) // pointer to Node or output
        {
            Nodlang::get_instance().serialize_node( source_code, property->owner());
        }
        else
        {
            Nodlang::get_instance().serialize_property(source_code, property);
        }

        ImGui::Text("source: \"%s\"", source_code.c_str());

        fw::ImGuiEx::EndTooltip();
    }

    // compute center position
    ImVec2 pos = ImGui::GetItemRectMin() ;
    fw::ImGuiEx::DebugCircle(pos, 2.5f, ImColor(0,0,0));
    pos += ImGui::GetItemRectSize() * 0.5f;

    // memorize
    _view->position = pos;

    return changed;
}

bool NodeView::draw_property(Property* _property, const char *_label)
{
    bool  changed = false;
    Node* node    = _property->owner().get();

    // Create a label (everything after ## will not be displayed)
    std::string label;
    if ( _label != nullptr )
    {
        label.append(_label);
    }
    else
    {
        label.append("##" + _property->get_name());
    }

    auto inputFlags = ImGuiInputTextFlags_None;

    if( const VariableNode* variable = _property->get_connected_variable() ) // if is a ref to a variable, we just draw variable name
    {
        char str[255];
        snprintf(str, 255, "%s", variable->name.c_str() );

        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4) variable->get_component<NodeView>()->get_color(Color_FILL) );
        ImGui::InputText(label.c_str(), str, 255, inputFlags);
        ImGui::PopStyleColor();

    }
    else if( !_property->value()->is_initialized() )
    {
        ImGui::LabelText(label.c_str(), "uninitialized");
    }
    else
    {
        /* Draw the property */
        const fw::type* t = _property->get_type();

        if(t->is<i16_t>() )
        {
            auto i16 = (i16_t)*_property->value();

            if (ImGui::InputInt(label.c_str(), &i16, 0, 0, inputFlags ) && !_property->has_input_connected())
            {
                _property->set(i16);
                changed |= true;
            }
        }
        else if(t->is<double>() )
        {
            auto d = (double)*_property->value();

            if (ImGui::InputDouble(label.c_str(), &d, 0.0F, 0.0F, "%g", inputFlags ) && !_property->has_input_connected())
            {
                _property->set(d);
                changed |= true;
            }
        }
        else if(t->is<std::string>() )
        {
            char str[255];
            snprintf(str, 255, "%s", (const char*)*_property->value() );

            if ( ImGui::InputText(label.c_str(), str, 255, inputFlags) && !_property->has_input_connected() )
            {
                _property->set( std::string(str) );
                changed |= true;
            }
        }
        else if(t->is<bool>() )
        {
            std::string checkBoxLabel = _property->get_name();

            auto b = (bool)*_property->value();

            if (ImGui::Checkbox(label.c_str(), &b ) && !_property->has_input_connected() )
            {
                _property->set(b);
                changed |= true;
            }
        }
        else
        {
            auto property_as_string = (*_property)->to<std::string>();
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

    auto drawProperty = [&](Property * _property)
    {
        // label (<name> (<way> <type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s, %s): ",
                _property->get_name().c_str(),
                to_string(_property->get_allowed_connection()),
                _property->get_type()->get_name());

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( fw::ImGuiEx::BeginTooltip() )
        {
            const auto variant = _property->value();
            ImGui::Text("initialized: %s,\n"
                        "defined:     %s,\n"
                        "Source token:\n"
                        "%s\n",
                        variant->is_initialized() ? "true" : "false",
                        variant->is_defined()     ? "true" : "false",
                        _property->token.json().c_str()
                        );
            fw::ImGuiEx::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool edited = NodeView::draw_property(_property, nullptr);
        node->dirty |= edited;

    };

    ImGui::Text("Name:       \"%s\"" , node->name.c_str());
    ImGui::Text("Class:      %s"     , node->get_type()->get_name());

    // Draw exposed input properties
    ImGui::Separator();
    ImGui::Text("Input(s):" );
    ImGui::Separator();
    ImGui::Indent();
    if( _view->m_exposed_input_only_properties.empty() )
    {
        ImGui::Text("None.");
        ImGui::Separator();
    }
    else
    {
        for (auto& property_view : _view->m_exposed_input_only_properties )
        {
            drawProperty(property_view->property() );
            ImGui::Separator();
        }
    }
    ImGui::Unindent();

    // Draw exposed output properties
    ImGui::Text("Output(s):" );
    ImGui::Separator();
    ImGui::Indent();
    if( _view->m_exposed_out_or_inout_properties.empty() )
    {
        ImGui::Text("None.");
        ImGui::Separator();
    }
    else
    {
        for (auto& eachView : _view->m_exposed_out_or_inout_properties )
        {
            drawProperty( eachView->property() );
            ImGui::Separator();
        }
    }
    ImGui::Unindent();

    if ( ImGui::TreeNode("Debug") )
    {
        // Draw exposed output properties
        if( ImGui::TreeNode("Other Properties") )
        {
            drawProperty( _view->m_exposed_this_property_view->property() );
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

            auto draw_slots = [](const char *label, const SlotBag& slots)
            {
                if( ImGui::TreeNode(label) )
                {
                    if (!slots.empty())
                    {
                        for (PoolID<Node> each : slots )
                        {
                            ImGui::BulletText("- %s", each->name.c_str());
                        }
                    }
                    else
                    {
                        ImGui::BulletText("None");
                    }
                    ImGui::TreePop();
                }
            };

            draw_slots("Inputs:"      , node->inputs);
            draw_slots("Outputs:"     , node->outputs);
            draw_slots("Predecessors:", node->predecessors);
            draw_slots("Successors:"  , node->successors);
            draw_slots("Children:"    , node->children);

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

                if (Node* parent = node->parent.get())
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

bool NodeView::is_exposed(const Property *_property)const
{
    return m_exposed_properties.find(_property) != m_exposed_properties.end();
}

void NodeView::set_view_detail(NodeViewDetail _viewDetail)
{
    NodeView::s_view_detail = _viewDetail;

    for( auto& eachView : Pool::get_pool()->get_all<NodeView>())
    {
        for( auto& [_, property_view] : eachView.m_exposed_properties )
        {
            property_view->reset();
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

        if ( view->m_is_visible && !(view->pinned && _ignorePinned) && view->should_follow_output( this->m_id ) )
        {
            ImRect child_rect = view->get_rect(true, _ignorePinned, _ignoreMultiConstrained);
            fw::ImGuiEx::EnlargeToInclude(result_rect, child_rect);
        }
    };

    std::for_each(children.begin(), children.end(), enlarge_to_fit_all);
    std::for_each(inputs.begin()  , inputs.end()  , enlarge_to_fit_all);

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
    for(PoolID<NodeView> each_child_view : children )
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
    return outputs.empty() || outputs[0] == output;
}

void NodeView::set_inputs_visible(bool _visible, bool _recursive)
{
    for(PoolID<NodeView> each_id : inputs )
    {
        NodeView* each_input_view = each_id.get();
        if( _visible || (outputs.empty() || each_input_view->should_follow_output( this->m_id )) )
        {
            if ( _recursive && each_input_view->m_expanded ) // propagate only if expanded
            {
                each_input_view->set_children_visible(_visible, true);
                each_input_view->set_inputs_visible(_visible, true);
            }
            each_input_view->set_visible(_visible);
        }
    }
}

void NodeView::set_children_visible(bool _visible, bool _recursive)
{
    for( auto each_child_view : children )
    {
        if( _visible || (outputs.empty() || each_child_view->should_follow_output( this->m_id )) )
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

    Node* parent = _view->m_owner.get()->parent.get();
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

ImRect NodeView::get_screen_rect()
{
    ImVec2 half_size = m_size / 2.0f;
    ImVec2 screen_pos = get_position(fw::Space_Screen, false);
    return {screen_pos - half_size, screen_pos + half_size};
}

bool NodeView::is_any_selected()
{
    return NodeView::get_selected().get() != nullptr;
}

SlotView& NodeView::get_slot_view(ID<Slot> _id)
{
    return m_slot_view.at(_id.m_value);
}