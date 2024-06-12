#include "SlotView.h"
#include "Config.h"
#include "Event.h"
#include "NodeView.h"
using namespace ndbl;
using namespace tools;

SlotView::SlotView(
    PoolID<NodeView> parent,
    Slot& slot,
    const Vec2& align,
    ShapeType shape
    )
: tools::View(parent.get())
, m_slot(slot)
, m_align(align)
, m_parent(parent)
, m_shape(shape)
{
}

PoolID<Node> SlotView::adjacent_node() const
{
   return m_slot.first_adjacent().node;
}

PoolID<Node> SlotView::get_node()const
{
    return m_slot.node;
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

tools::Vec2 SlotView::normal() const
{
    return tools::Vec2::normalize( m_align );
}

tools::string64 SlotView::compute_tooltip() const
{
    switch (slot().type_and_order())
    {
        case SlotFlag_NEXT:   return "next";
        case SlotFlag_PREV:   return "previous";
        case SlotFlag_PARENT: return "parent";
        case SlotFlag_CHILD:  return "children";
        default:
            const std::string &prop_name = get_property()->get_name();
            ASSERT(prop_name.length() < 59)
            string64 result;
            switch (slot().type_and_order())
            {
                case SlotFlag_INPUT:  result.append_fmt("%s (in)",  prop_name.c_str());  break;
                case SlotFlag_OUTPUT: result.append_fmt("%s (out)", prop_name.c_str());
            }
            return result;
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
    Vec2 pos  = rect.center();

    switch (m_shape)
    {
        case ShapeType_CIRCLE:
        {
            float invisible_ratio = cfg->ui_slot_invisible_ratio;
            float radius = cfg->ui_slot_radius;
            float diameter = radius * invisible_ratio;

            // move cursor to slot to circle's top-left corner bbox
            // TODO: we should be using SlotView's rect top left corner instead
            Vec2 cursor_pos{ pos - Vec2{diameter}};
            ImGui::SetCursorScreenPos( cursor_pos);

            // draw an invisible button (for easy mouse interaction)
            ImGui::PushID((u8_t)m_slot.id);
            ImGui::InvisibleButton("###", Vec2{diameter * invisible_ratio});
            ImGui::PopID();

            // draw the circle
            Vec4 fill_color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? hover_color : color;
            draw_list->AddCircleFilled(pos, radius, ImColor(fill_color));
            draw_list->AddCircle(pos, radius, ImColor(border_color) );
            ImGuiEx::DebugCircle(pos, radius, ImColor(border_color));
            break;
        }
        case ShapeType_RECTANGLE:
        {
            // draw an invisible button (for easy mouse interaction)
            ImGui::SetCursorScreenPos(rect.tl());
            ImGui::PushID((u8_t)m_slot.id);
            ImGui::InvisibleButton("###", rect.size());
            ImGui::PopID();

            // draw the rectangle
            bool bottom = m_slot.has_flags(SlotFlag_ORDER_FIRST);
            ImDrawCornerFlags corner_flags = bottom ? ImDrawCornerFlags_Bot
                                                    : ImDrawCornerFlags_Top;
            Vec4 fill_color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? hover_color : color;
            draw_list->AddRectFilled(rect.min, rect.max, ImColor(fill_color), border_radius, corner_flags );
            draw_list->AddRect(rect.min, rect.max, ImColor(border_color), border_radius, corner_flags );
            ImGuiEx::DebugRect(rect.min, rect.max, ImColor(255, 0, 0, 127), 0.0f );
            break;
        }
        default:
            EXPECT(false, "Unhandled case")
    }

    hovered = ImGui::IsItemHovered() ;

    if ( ImGuiEx::BeginTooltip() )
    {
        ImGui::Text("%s", compute_tooltip().c_str() );
        ImGuiEx::EndTooltip();
    }

    return ImGui::IsItemClicked();
};