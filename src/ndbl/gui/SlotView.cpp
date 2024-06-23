#include "SlotView.h"
#include "Config.h"
#include "Event.h"
#include "NodeView.h"

using namespace ndbl;
using namespace tools;

SlotView::SlotView(
    Slot*       slot,
    const Vec2& align,
    ShapeType   shape,
    size_t      index
    )
: tools::View()
, m_slot(slot)
, m_align(align)
, m_shape(shape)
, m_index(index)
{
    ASSERT(slot != nullptr)
    slot->set_view(this);
}

Node* SlotView::adjacent_node() const
{
   return m_slot->first_adjacent()->get_node();
}

Node* SlotView::get_node()const
{
    return m_slot->get_node();
}

const type* SlotView::get_property_type()const
{
    Property* property = get_property();
    return property ? property->get_type() : nullptr;
}

bool SlotView::is_this() const
{
    return get_property()->is_this();
}

bool SlotView::allows(SlotFlag flags) const
{
    return m_slot->has_flags(flags);
}

Slot& SlotView::get_slot() const
{
    return *m_slot;
}

bool SlotView::has_node_connected() const
{
    if ( !m_slot->get_property()->get_type()->is<Node*>() )
    {
        return false;
    }

    return m_slot->adjacent_count() != 0;
}

Property* SlotView::get_property() const
{
    return m_slot->get_property();
}

tools::Vec2 SlotView::get_normal() const
{
    return tools::Vec2::normalize( m_align );
}

const tools::Vec2& SlotView::get_align() const
{
    return m_align;
}

tools::string64 SlotView::compute_tooltip() const
{
    switch (get_slot().type_and_order())
    {
        case SlotFlag_NEXT:   return "next";
        case SlotFlag_PREV:   return "previous";
        case SlotFlag_PARENT: return "parent";
        case SlotFlag_CHILD:  return "children";
        default:
            const std::string &prop_name = get_property()->get_name();
            ASSERT(prop_name.length() < 59)
            string64 result;
            switch (get_slot().type_and_order())
            {
                case SlotFlag_INPUT:  result.append_fmt("%s (in)",  prop_name.c_str());  break;
                case SlotFlag_OUTPUT: result.append_fmt("%s (out)", prop_name.c_str());
            }
            return std::move(result);
    }
}

bool SlotView::draw()
{
    if( !visible )
    {
        return false;
    };

    View::draw();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    Config* cfg          = get_config();
    Vec4  color          = cfg->ui_slot_color;
    Vec4  border_color   = cfg->ui_slot_border_color;
    float border_radius  = cfg->ui_slot_border_radius;
    Vec4  hover_color    = cfg->ui_slot_hovered_color;

    Rect rect = get_rect(SCREEN_SPACE);

    if ( !rect.has_area() )
        return false;

    Vec2 pos  = rect.center();

    // draw an invisible button (for easy mouse interaction)
    ImGui::SetCursorScreenPos(rect.top_left());
    ImGui::PushID(m_slot);
    ImGui::InvisibleButton("###", rect.size() * cfg->ui_slot_invisible_ratio);
    ImGui::PopID();
    hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // draw shape
    switch (m_shape)
    {
        case ShapeType_CIRCLE:
        {
            float r = cfg->ui_slot_circle_radius();

            Vec4 fill_color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? hover_color : color;
            draw_list->AddCircleFilled(pos, r, ImColor(fill_color));
            draw_list->AddCircle(pos, r, ImColor(border_color) );
            break;
        }
        case ShapeType_RECTANGLE:
        {
            // draw the rectangle
            bool bottom = m_slot->has_flags(SlotFlag_ORDER_FIRST);
            ImDrawCornerFlags corner_flags = bottom ? ImDrawCornerFlags_Bot
                                                    : ImDrawCornerFlags_Top;
            hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
            Vec4 fill_color = hovered ? hover_color : color;
            draw_list->AddRectFilled(rect.min, rect.max, ImColor(fill_color), border_radius, corner_flags );
            draw_list->AddRect(rect.min, rect.max, ImColor(border_color), border_radius, corner_flags );
            break;
        }
        default:
            EXPECT(false, "Unhandled case")
    }

    if ( ImGuiEx::BeginTooltip() )
    {
        string64 tooltip = compute_tooltip();
        ImGui::Text("%s", tooltip.c_str() );
        ImGuiEx::EndTooltip();
    }

    return ImGui::IsItemClicked();
}

size_t SlotView::get_index() const
{
    return m_index;
}

ShapeType SlotView::get_shape() const
{
    return m_shape;
};