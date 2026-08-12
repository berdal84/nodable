#include "Node_Slot_View.h"
#include "Config.h"
#include "Event.h"
#include "core/Flags.h"
#include "gui/ImGuiEx.h"
#include "gui/View_Flags.h"
#include "gui/geometry/Vec2.h"
#include "ndbl/core/Node.h"

using namespace ndbl;
using namespace tools;

Node_Slot_View::Node_Slot_View(
    Node_Slot*      slot,
    const Vec2&     align,
    Shape_Type      shape_type,
    size_t          index,
    const Box_2D*   alignment_ref
)
: slot(slot)
, alignment_pivot(align)
, shape_type(shape_type)
, index(index)
, alignment_ref(alignment_ref)
, direction()
, shape(Vec2{1.f, 1.f})
, flags(0)
{
    ASSERT(slot != nullptr);

    slot->view = this;
    nodeslotview_update_direction_from_alignment(this);

    // Update size from shape
    Config* config = get_config();
    Vec2 size = shape_type == Shape_Type_CIRCLE
            ? Vec2{ config->ui_slot_circle_radius() * 2.f}
            : config->ui_slot_rectangle_size;
    shape.set_size( size );
}

String_64 ndbl::nodeslotview_compute_tooltip(const Node_Slot_View* view)
{
    switch (view->slot->type_and_order())
    {
        case Node_Slot::Flag_FLOW_OUT: return "flow_out";
        case Node_Slot::Flag_FLOW_IN:  return "flow_in";
    }

    std::string prop_name;

    if ( view->property() )
        prop_name = view->property()->name;

    String_64 result;
    switch (view->slot->type_and_order())
    {
        case Node_Slot::Flag_INPUT:  result.append_fmt("%s (in)",  prop_name.c_str());  break;
        case Node_Slot::Flag_OUTPUT: result.append_fmt("%s (out)", prop_name.c_str());
    }

    return std::move(result);
}

bool ndbl::nodeslotview_draw(Node_Slot_View* view)
{
    box2d_draw_debug_info(&view->shape);

    if ( HAS_FLAGS(view->flags, View_Flag_HIDDEN) )
    {
        return false;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    Config* cfg          = get_config();
    Vec4  color          = cfg->ui_slot_color(view->slot->flags );
    Vec4  border_color   = cfg->ui_slot_border_color;
    float border_radius  = cfg->ui_slot_border_radius;
    Vec4  hover_color    = cfg->ui_slot_hovered_color;
    Rect rect            = view->shape.rect(WORLD_SPACE);

    if ( !rect.has_area() )
        return false;

    // draw an invisible button (for easy mouse interaction)
    ImGui::SetCursorScreenPos(rect.top_left());
    ImGui::PushID(view->slot);
    ImGui::InvisibleButton("###", rect.size() + cfg->ui_slot_invisible_btn_expand_size);
    ImGui::PopID();
    const bool hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);
    SET_FLAGS_VALUE(view->flags, View_Flag_HOVERED, hovered);
    const Vec4 fill_color = hovered ? hover_color : color;

    // draw shape
    switch ( view->shape_type )
    {
        case Shape_Type_CIRCLE:
        {
            const float radius = view->shape.size.x / 2.f;
            draw_list->AddCircleFilled( rect.center(), radius, ImColor(fill_color));
            draw_list->AddCircle( rect.center(), radius, ImColor(border_color) );
            break;
        }
        case Shape_Type_RECTANGLE:
        {
            // draw the rectangle
            bool bottom = view->slot->has_flags(Node_Slot::Flag_ORDER_1ST);
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
        String_64 tooltip = nodeslotview_compute_tooltip(view);
        ImGui::Text("%s", tooltip.c_str() );
        ImGuiEx::EndTooltip();
    }

    return ImGui::IsItemClicked();
}

void ndbl::nodeslotview_update(Node_Slot_View* view, float dt)
{
    // 1) Update visibility
    //---------------------

    if (view->slot->capacity == 0)
    {
        SET_FLAGS(view->flags, View_Flag_HIDDEN);
    }
    else if (view->slot->type() == Node_Slot::Flag_TYPE_FLOW )
    {
        // A code flow slot has to be hidden when cannot be an instruction or is not
        bool desired_visibility = node_is_instruction(view->node() ) || node_could_be_instruction(view->node() );
        SET_FLAGS_VALUE(view->flags, View_Flag_HIDDEN, !desired_visibility );
    }
    else
    {
        UNSET_FLAGS(view->flags, View_Flag_HIDDEN);
    }

    // 2) Update position
    //-------------------

    const Config* cfg = get_config();
    if (view->slot->type() == Node_Slot::Flag_TYPE_FLOW )
    {
        // Align the code flow slots like that (example at top-left corner)
        //
        // [0][1]...[n-1]
        // ---------------------
        // |  Box              |
        // ---------------------
        //
        const Vec2  size  = view->shape.size;
        const float gap   = cfg->ui_slot_gap;
        const float dir_x = 1.f;

        const Vec2 pos = view->alignment_ref->pivot_position(view->alignment_pivot, WORLD_SPACE )
                       + Vec2( dir_x * gap * float(view->index + 2), 0.f) // horizontal gaps (2 initial, then 1 per slot)
                       + Vec2( dir_x * size.x * float(view->index), 0.f) // jump to index
                       + Vec2(0.f, view->alignment_pivot.y * size.y * 0.5f) // align edge vertically
                       - size / 2.0f;

        spatialnode_set_position(&view->shape.spatial_node, pos, WORLD_SPACE); // relative to Node_View's
    }
    else if (view->alignment_ref != nullptr )
    {
        const float radius = view->shape.size.x / 2.0f;
        const Vec2 pos  = view->alignment_ref->pivot_position( view->alignment_pivot, WORLD_SPACE)
                        - Vec2(radius);
        spatialnode_set_position(&view->shape.spatial_node, pos, WORLD_SPACE);
    }
    else
    {
        // positioned manually
    }
}

void ndbl::nodeslotview_update_direction_from_alignment(Node_Slot_View* view)
{
    view->direction = Vec2::normalize( view->alignment_pivot - 0.5f );
}
