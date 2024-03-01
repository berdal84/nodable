#include "SlotView.h"
#include "NodeView.h"
#include "Event.h"
#include "Nodable.h"

using namespace ndbl;

SlotView *SlotView::s_focused = nullptr;
SlotView *SlotView::s_dragged = nullptr;
SlotView *SlotView::s_hovered = nullptr;

SlotView::SlotView( Slot &_slot, ImVec2 _alignment )
: m_slot(_slot)
, m_alignment(_alignment)
{
}

void SlotView::draw_slot_circle(
        ImDrawList* _draw_list,
        SlotView& _view,
        ImVec2 _position,
        float _radius,
        const ImColor& _color,
        const ImColor& _border_color,
        const ImColor& _hover_color,
        bool _readonly)
{
    constexpr float INVISIBLE_BUTTON_SIZE_RATIO = 1.2f; // 120%

    // draw
    //-----
    ImGui::SetCursorScreenPos( _position - ImVec2(_radius * INVISIBLE_BUTTON_SIZE_RATIO ));

    // draw a larger invisible button on top of the circle to facilitate click/drag
    ImGui::PushID((u8_t)_view.m_slot.id);
    ImGui::InvisibleButton("###", ImVec2(_radius * 2.0f * INVISIBLE_BUTTON_SIZE_RATIO, _radius * 2.0f * INVISIBLE_BUTTON_SIZE_RATIO ));
    ImGui::PopID();

    bool is_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // draw the circle
    _draw_list->AddCircleFilled( _position, _radius, is_hovered ? _hover_color : _color);
    _draw_list->AddCircle( _position, _radius, _border_color );
    fw::ImGuiEx::DebugCircle( _position, _radius, _border_color, ImColor(255, 0, 0, 0));

    behavior(_view, _readonly);
}

void SlotView::draw_slot_rectangle(
        ImDrawList* _draw_list,
        SlotView& _view,
        ImRect _rect,
        const ImColor& _color,
        const ImColor& _border_color,
        const ImColor& _hover_color,
        float _border_radius,
        bool _readonly)
{
    ImVec2 rect_size = _rect.GetSize();

    // Return early if rectangle cannot be draw.
    // TODO: Find why size can be zero more (more surprisingly) nan.
    if(rect_size.x == 0.0f || rect_size.y == 0.0f || std::isnan(rect_size.x) || std::isnan(rect_size.y) ) return;

    ImDrawCornerFlags corner_flags = _view.m_slot.has_flags(SlotFlag_ORDER_FIRST) ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_Top;

    ImGui::SetCursorScreenPos( _rect.GetTL());
    ImGui::PushID((u8_t)_view.m_slot.id);
    ImGui::InvisibleButton("###", _rect.GetSize());
    ImGui::PopID();

    ImColor fill_color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? _hover_color : _color;
    _draw_list->AddRectFilled( _rect.Min, _rect.Max, fill_color, _border_radius, corner_flags );
    _draw_list->AddRect( _rect.Min, _rect.Max, _border_color, _border_radius, corner_flags );
    fw::ImGuiEx::DebugRect( _rect.Min, _rect.Max, ImColor(255,0, 0, 127), 0.0f );

    behavior(_view, _readonly);
}

void SlotView::behavior(SlotView& _view, bool _readonly)
{
    // Handle disconnect

    if ( !_readonly && _view.has_node_connected() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            fw::EventManager::get_instance().dispatch<SlotDisconnectedEvent>( { _view.m_slot } );
        }

        ImGui::EndPopup();
    }

    // Handle hover state

    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        SlotView::reset_hovered(&_view);
        if( fw::ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("%s", SlotView::get_tooltip(_view).c_str() );
            fw::ImGuiEx::EndTooltip();
        }

        if ( ImGui::IsMouseDown(0) && !is_dragging() && !NodeView::is_any_dragged())
        {
            if ( !_view.slot().is_full() )
            {
                reset_dragged( &_view );
            }
        }
    }
    else if (SlotView::get_hovered() == &_view)
    {
        SlotView::reset_hovered();
    }

}

PoolID<Node> SlotView::adjacent_node() const
{
   return m_slot.first_adjacent().node;
}

PoolID<Node> SlotView::get_node()const
{
    return m_slot.node;
}

ImVec2 SlotView::position()const
{
    return m_slot.node->get_component<NodeView>()->get_slot_pos( m_slot );
}

ImRect SlotView::rect(Config& config)const
{
    return m_slot.node->get_component<NodeView>()->get_slot_rect( *this, config, 0 );
}

void SlotView::drop_behavior(bool &require_new_node, bool _enable_edition)
{
    if ( s_dragged && ImGui::IsMouseReleased(0) )
    {
        if ( _enable_edition )
        {
            if ( s_hovered )
            {
                SlotEventPayload payload;
                payload.first  = s_dragged->m_slot;
                payload.second = s_hovered->m_slot;
                fw::EventManager::get_instance().dispatch<SlotDroppedEvent>(payload);

                reset_hovered();
                reset_dragged();
            }
            else
            {
                require_new_node = true;
            }
        }
        else
        {
            reset_dragged();
        }
    }
}

const fw::type* SlotView::get_property_type()const
{
    Property* property = get_property();
    return property ? property->get_type() : nullptr;
}

ImVec2 SlotView::alignment() const
{
    return m_alignment;
}

bool SlotView::is_this() const
{
    return get_property()->is_this();
}

bool SlotView::allows(SlotFlag flags) const
{
    return ( m_slot.flags & flags ) == flags;
}

Slot& SlotView::slot() const
{
    return m_slot;
}

bool SlotView::has_node_connected() const
{
    if ( !m_slot.get_property()->get_type()->is<PoolID<Node>>() )
    {
        return false;
    }

    return m_slot.adjacent_count() != 0;
}

Property* SlotView::get_property() const
{
    return m_slot.get_property();
}

std::string SlotView::get_tooltip( SlotView& _view )
{
    std::string property_name{_view.get_property()->get_name()};

    switch ( _view.slot().static_flags() ) // type and order flags only
    {
        case SlotFlag_INPUT:   return property_name.append(" (in)");
        case SlotFlag_OUTPUT:  return property_name.append(" (out)");
        case SlotFlag_NEXT:    return "next";
        case SlotFlag_PREV:    return "previous";
        case SlotFlag_PARENT:  return "parent";
        case SlotFlag_CHILD:   return "children";
    }
    return property_name;
}
