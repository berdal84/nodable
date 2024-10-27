#include "SlotView.h"
#include "Config.h"
#include "Event.h"
#include "ndbl/core/Utils.h"

using namespace ndbl;
using namespace tools;

SlotView::SlotView(
    Slot*       slot,
    const Vec2& align,
    ShapeType   shape,
    size_t      index,
    const BoxShape2D* alignment_ref
    )
: slot(slot)
, alignment(align)
, shape(shape)
, index(index)
, alignment_ref(alignment_ref)
, direction()
{
    ASSERT(slot != nullptr);

    slot->view = this;
    update_direction_from_alignment();

    // Update size from shape
    Config* config = get_config();
    Vec2 size = shape == ShapeType_CIRCLE
            ? Vec2{ config->ui_slot_circle_radius() }
            : config->ui_slot_rectangle_size;
    box()->set_size( size );
}

string64 SlotView::compute_tooltip() const
{
    switch (slot->type_and_order())
    {
        case SlotFlag_FLOW_OUT: return "flow_out";
        case SlotFlag_FLOW_IN:  return "flow_in";
    }

    std::string prop_name;

    if (property() )
        prop_name = property()->name();

    string64 result;
    switch (slot->type_and_order())
    {
        case SlotFlag_INPUT:  result.append_fmt("%s (in)",  prop_name.c_str());  break;
        case SlotFlag_OUTPUT: result.append_fmt("%s (out)", prop_name.c_str());
    }

    return std::move(result);
}

bool SlotView::draw()
{
    state.box.draw_debug_info();

    if ( !state.visible )
        return false;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    Config* cfg          = get_config();
    Vec4  color          = cfg->ui_slot_color(slot->flags() );
    Vec4  border_color   = cfg->ui_slot_border_color;
    float border_radius  = cfg->ui_slot_border_radius;
    Vec4  hover_color    = cfg->ui_slot_hovered_color;
    Rect rect            = state.box.get_rect(WORLD_SPACE );

    if ( !rect.has_area() )
        return false;

    // draw an invisible button (for easy mouse interaction)
    ImGui::SetCursorScreenPos(rect.top_left());
    ImGui::PushID(slot);
    ImGui::InvisibleButton("###", rect.size() + cfg->ui_slot_invisible_btn_expand_size);
    ImGui::PopID();
    state.hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
    Vec4 fill_color = state.hovered ? hover_color : color;

    // draw shape
    switch (shape)
    {
        case ShapeType_CIRCLE:
        {
            float r = box()->size().x;
            draw_list->AddCircleFilled( rect.center(), r, ImColor(fill_color));
            draw_list->AddCircle( rect.center(), r, ImColor(border_color) );
            break;
        }
        case ShapeType_RECTANGLE:
        {
            // draw the rectangle
            bool bottom = slot->has_flags(SlotFlag_ORDER_1ST);
            ImDrawCornerFlags corner_flags = bottom ? ImDrawCornerFlags_Bot
                                                    : ImDrawCornerFlags_Top;
            draw_list->AddRectFilled(rect.min, rect.max, ImColor(fill_color), border_radius, corner_flags );
            draw_list->AddRect(rect.min, rect.max, ImColor(border_color), border_radius, corner_flags );
            break;
        }
        default:
            VERIFY(false, "Unhandled case");
    }

    if ( ImGuiEx::BeginTooltip() )
    {
        string64 tooltip = compute_tooltip();
        ImGui::Text("%s", tooltip.c_str() );
        ImGuiEx::EndTooltip();
    }

    return ImGui::IsItemClicked();
}

void SlotView::update(float dt)
{
    // 1) Update visibility
    //---------------------

    if (slot->capacity() == 0)
    {
        state.visible = false;
    }
    else if (slot->type() == SlotFlag_TYPE_FLOW )
    {
        // A code flow slot has to be hidden when cannot be an instruction or is not
        bool desired_visibility = Utils::is_instruction( node() ) || Utils::can_be_instruction( node() );
        state.visible = desired_visibility;
    }
    else
    {
        state.visible = true;
    }

    // 2) Update position
    //-------------------

    const Config* cfg = get_config();
    if (slot->type() == SlotFlag_TYPE_FLOW )
    {
        // Align the code flow slots like that (example at top-left corner)
        //
        // [0][1]...[n-1]
        // ---------------------
        // |  Box              |
        // ---------------------
        //
        const Vec2  size  = box()->size();
        const float gap   = cfg->ui_slot_gap;
        const float dir_x = -alignment.x;

        const Vec2 pos = alignment_ref->pivot(alignment, WORLD_SPACE ) // Starts from the right corner
                       + Vec2( dir_x * gap * float(index + 2), 0.f) // horizontal gaps (2 initial, then 1 per slot)
                       + Vec2( dir_x * size.x * float(index), 0.f) // jump to index
                       + Vec2(0.f, alignment.y * size.y * 0.5f); // align edge vertically

        box()->xform.set_pos( pos, WORLD_SPACE ); // relative to NodeView's
    }
    else if (alignment_ref != nullptr )
    {
        const Vec2 pos  = alignment_ref->pivot( alignment, WORLD_SPACE);
        box()->xform.set_pos( pos, WORLD_SPACE );
    }
    else
    {
        // positioned manually
    }
}

void SlotView::update_direction_from_alignment()
{
    direction = Vec2::normalize( alignment );
}
