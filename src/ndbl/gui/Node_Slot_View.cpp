#include "Node_Slot_View.h"
#include "Config.h"
#include "Event.h"
#include "ndbl/core/Node.h"

using namespace ndbl;
using namespace tools;

Node_Slot_View::Node_Slot_View(
        Node_Slot*       slot,
        const Vec2& align,
        Shape_Type   shape_type,
        size_t      index,
        const Box_2D* alignment_ref
    )
: slot(slot)
, alignment(align)
, shape_type(shape_type)
, index(index)
, alignment_ref(alignment_ref)
, direction()
, _shape(Vec2{1.f, 1.f})
, _state()
{
    ASSERT(slot != nullptr);

    slot->view = this;
    update_direction_from_alignment();

    // Update size from shape
    Config* config = get_config();
    Vec2 size = shape_type == Shape_Type_CIRCLE
            ? Vec2{ config->ui_slot_circle_radius() }
            : config->ui_slot_rectangle_size;
    _shape.set_size( size );
}

String_64 Node_Slot_View::compute_tooltip() const
{
    switch (slot->type_and_order())
    {
        case Node_Slot_Flag_FLOW_OUT: return "flow_out";
        case Node_Slot_Flag_FLOW_IN:  return "flow_in";
    }

    std::string prop_name;

    if ( property() )
        prop_name = property()->name;

    String_64 result;
    switch (slot->type_and_order())
    {
        case Node_Slot_Flag_INPUT:  result.append_fmt("%s (in)",  prop_name.c_str());  break;
        case Node_Slot_Flag_OUTPUT: result.append_fmt("%s (out)", prop_name.c_str());
    }

    return std::move(result);
}

bool Node_Slot_View::draw()
{
    _shape.draw_debug_info();

    if ( !_state.visible() )
        return false;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    Config* cfg          = get_config();
    Vec4  color          = cfg->ui_slot_color(slot->flags() );
    Vec4  border_color   = cfg->ui_slot_border_color;
    float border_radius  = cfg->ui_slot_border_radius;
    Vec4  hover_color    = cfg->ui_slot_hovered_color;
    Rect rect            = _shape.rect(WORLD_SPACE);

    if ( !rect.has_area() )
        return false;

    // draw an invisible button (for easy mouse interaction)
    ImGui::SetCursorScreenPos(rect.top_left());
    ImGui::PushID(slot);
    ImGui::InvisibleButton("###", rect.size() + cfg->ui_slot_invisible_btn_expand_size);
    ImGui::PopID();
    bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
    _state.set_hovered( hovered );
    const Vec4 fill_color = hovered ? hover_color : color;

    // draw shape
    switch ( shape_type )
    {
        case Shape_Type_CIRCLE:
        {
            float r = _shape.size().x;
            draw_list->AddCircleFilled( rect.center(), r, ImColor(fill_color));
            draw_list->AddCircle( rect.center(), r, ImColor(border_color) );
            break;
        }
        case Shape_Type_RECTANGLE:
        {
            // draw the rectangle
            bool bottom = slot->has_flags(Node_Slot_Flag_ORDER_1ST);
            ImDrawFlags corner_flags = bottom ? ImDrawFlags_RoundCornersBottom
                                              : ImDrawFlags_RoundCornersTop;
            draw_list->AddRectFilled(rect.min, rect.max, ImColor(fill_color), border_radius, corner_flags );
            draw_list->AddRect(rect.min, rect.max, ImColor(border_color), border_radius, corner_flags );
            break;
        }
        default:
            VERIFY(false, "Unhandled case");
    }

    if ( ImGuiEx::BeginTooltip() )
    {
        String_64 tooltip = compute_tooltip();
        ImGui::Text("%s", tooltip.c_str() );
        ImGuiEx::EndTooltip();
    }

    return ImGui::IsItemClicked();
}

void Node_Slot_View::update(float dt)
{
    // 1) Update visibility
    //---------------------

    if (slot->capacity() == 0)
    {
        _state.set_visible(false);
    }
    else if (slot->type() == Node_Slot_Flag_TYPE_FLOW )
    {
        // A code flow slot has to be hidden when cannot be an instruction or is not
        bool desired_visibility = node_is_instruction(node() ) || node_could_be_instruction(node() );
        _state.set_visible( desired_visibility );
    }
    else
    {
        _state.set_visible(true);
    }

    // 2) Update position
    //-------------------

    const Config* cfg = get_config();
    if (slot->type() == Node_Slot_Flag_TYPE_FLOW )
    {
        // Align the code flow slots like that (example at top-left corner)
        //
        // [0][1]...[n-1]
        // ---------------------
        // |  Box              |
        // ---------------------
        //
        const Vec2  size  = _shape.size();
        const float gap   = cfg->ui_slot_gap;
        const float dir_x = -alignment.x;

        const Vec2 pos = alignment_ref->pivot(alignment, WORLD_SPACE ) // Starts from the right corner
                       + Vec2( dir_x * gap * float(index + 2), 0.f) // horizontal gaps (2 initial, then 1 per slot)
                       + Vec2( dir_x * size.x * float(index), 0.f) // jump to index
                       + Vec2(0.f, alignment.y * size.y * 0.5f); // align edge vertically

        spatial_node()->set_position(pos, WORLD_SPACE); // relative to Node_View's
    }
    else if (alignment_ref != nullptr )
    {
        const Vec2 pos  = alignment_ref->pivot( alignment, WORLD_SPACE);
        spatial_node()->set_position(pos, WORLD_SPACE);
    }
    else
    {
        // positioned manually
    }
}

void Node_Slot_View::update_direction_from_alignment()
{
    direction = Vec2::normalize( alignment );
}
