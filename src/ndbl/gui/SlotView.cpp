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
    size_t      index,
    const BoxShape2D* alignment_ref
    )
: m_slot(slot)
, m_alignment(align)
, m_shape(shape)
, m_index(index)
, m_alignment_ref(alignment_ref)
{
    ASSERT(slot != nullptr)
    slot->set_view(this);
}

Node* SlotView::adjacent_node() const
{
   return m_slot->first_adjacent()->node();
}

Node* SlotView::node()const
{
    return m_slot->node();
}

const TypeDescriptor* SlotView::property_type()const
{
    if ( Property* p = property() )
        return p->get_type();
    return nullptr;
}

bool SlotView::is_this() const
{
    return property()->has_flags(PropertyFlag_IS_THIS);
}

bool SlotView::allows(SlotFlag flags) const
{
    return m_slot->has_flags(flags);
}

Slot& SlotView::slot() const
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

Property* SlotView::property() const
{
    return m_slot->get_property();
}

tools::Vec2 SlotView::normal() const
{
    return tools::Vec2::normalize(m_alignment );
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
            std::string prop_name;

            if (property() )
                prop_name = property()->name();

            string64 result;
            switch (slot().type_and_order())
            {
                case SlotFlag_INPUT:  result.append_fmt("%s (in)",  prop_name.c_str());  break;
                case SlotFlag_OUTPUT: result.append_fmt("%s (out)", prop_name.c_str());
            }
            return std::move(result);
    }
}

bool SlotView::draw()
{
    m_view_state.box.draw_debug_info();

    if ( !m_view_state.visible )
        return false;


    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    Config* cfg          = get_config();
    Vec4  color          = cfg->ui_slot_color;
    Vec4  border_color   = cfg->ui_slot_border_color;
    float border_radius  = cfg->ui_slot_border_radius;
    Vec4  hover_color    = cfg->ui_slot_hovered_color;
    Rect rect            = m_view_state.box.get_rect(WORLD_SPACE );

    if ( !rect.has_area() )
        return false;

    // draw an invisible button (for easy mouse interaction)
    ImGui::SetCursorScreenPos(rect.top_left());
    ImGui::PushID(m_slot);
    ImGui::InvisibleButton("###", rect.size() * cfg->ui_slot_invisible_ratio);
    ImGui::PopID();
    m_view_state.hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
    Vec4 fill_color = m_view_state.hovered ? hover_color : color;

    // draw shape
    switch (m_shape)
    {
        case ShapeType_CIRCLE:
        {
            float r = cfg->ui_slot_circle_radius();
            draw_list->AddCircleFilled( rect.center(), r, ImColor(fill_color));
            draw_list->AddCircle( rect.center(), r, ImColor(border_color) );
            break;
        }
        case ShapeType_RECTANGLE:
        {
            // draw the rectangle
            bool bottom = m_slot->has_flags(SlotFlag_ORDER_FIRST);
            ImDrawCornerFlags corner_flags = bottom ? ImDrawCornerFlags_Bot
                                                    : ImDrawCornerFlags_Top;
            draw_list->AddRectFilled(rect.min, rect.max, ImColor(fill_color), border_radius, corner_flags );
            draw_list->AddRect(rect.min, rect.max, ImColor(border_color), border_radius, corner_flags );
            break;
        }
        default:
            VERIFY(false, "Unhandled case")
    }

    if ( ImGuiEx::BeginTooltip() )
    {
        string64 tooltip = compute_tooltip();
        ImGui::Text("%s", tooltip.c_str() );
        ImGuiEx::EndTooltip();
    }

    return ImGui::IsItemClicked();
}

size_t SlotView::index() const
{
    return m_index;
}

ShapeType SlotView::shape() const
{
    return m_shape;
}

ViewState *SlotView::state()
{
    return &m_view_state;
}

bool SlotView::is_hovered() const
{
    return m_view_state.hovered;
}

void SlotView::set_align(const tools::Vec2 align)
{
    m_alignment = align;
}

void SlotView::set_shape(ShapeType shape)
{
    m_shape = shape;
}

void SlotView::set_align_ref(const tools::BoxShape2D* align_ref)
{
    m_alignment_ref = align_ref;
}

void SlotView::update(float dt)
{
    if ( m_slot->capacity() == 0)
    {
        m_view_state.visible = false;
    }
    else if (m_slot->type() == SlotFlag_TYPE_CODEFLOW )
    {
        // A code flow slot has to be hidden when cannot be an instruction or is not
        bool desired_visibility = node()->is_instruction() || node()->can_be_instruction();
        m_view_state.visible = desired_visibility;
    }
    else
    {
        m_view_state.visible = true;
    }

    if ( !m_view_state.visible )
        return;

    const Config* cfg = get_config();
    if ( m_slot->type() == SlotFlag_TYPE_CODEFLOW )
    {
        // Align the code flow slots like that (example at top-left corner)
        //
        // [0][1]...[n-1]
        // ---------------------
        // |  Box              |
        // ---------------------
        //
        const Vec2  size  = cfg->ui_slot_rectangle_size;
        const float gap   = cfg->ui_slot_gap;
        const float dir_x = -m_alignment.x;

        const Vec2 pos = m_alignment_ref->pivot(m_alignment, WORLD_SPACE ) // Starts from the right corner
                       + Vec2( dir_x * gap * float(m_index + 2), 0.f) // horizontal gaps (2 initial, then 1 per slot)
                       + Vec2( dir_x * size.x * float(m_index), 0.f) // jump to index
                       + Vec2(0.f, m_alignment.y * size.y * 0.5f); // align edge vertically

        box()->xform.set_pos( pos, WORLD_SPACE ); // relative to NodeView's
        box()->set_size( size );
    }
    else
    {
        // Align view
        const Vec2 size = cfg->ui_slot_circle_radius();
        const Vec2 pos  = m_alignment_ref->pivot( m_alignment, WORLD_SPACE);
        box()->xform.set_pos( pos, WORLD_SPACE );
        box()->set_size( size );
    }
}
