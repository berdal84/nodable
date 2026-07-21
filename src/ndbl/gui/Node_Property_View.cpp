#include "Node_Property_View.h"

#include "core/Asserts.h"
#include "gui/ImGuiEx.h"
#include "gui/geometry/Rect.h"
#include "gui/geometry/Space.h"
#include "gui/geometry/Vec2.h"
#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/Node.h"
#include "Node_View.h"
#include "Config.h"

using namespace ndbl;
using namespace tools;

constexpr Vec2  PROPERTY_TOGGLE_BTN_SIZE = Vec2(12.0, 22.0f);
constexpr float PROPERTY_INPUT_PADDING   = 5.0f;
constexpr float PROPERTY_INPUT_SIZE_MIN  = 12.0f;

Node_Property_View::Node_Property_View(Node_Property* _property )
: property(_property)
, show(false)
, touched(false)
, state()
, shape(Vec2{10.f, 10.f})
{
}

Node* Node_Property_View::node() const
{
    return property->node;
}

bool Node_Property_View::has_input_connected() const
{
    return node_has_input_connected(node(), property );
}

Node_Slot* Node_Property_View::connected_slot() const
{
    const Node_Slot* input_slot = node_find_slot_by_property(node(), property, Node_Slot::Flag_INPUT );
    if( !input_slot )
        return nullptr;

    return input_slot->first_adjacent();
}

Node* Node_Property_View::connected_variable() const
{
    Node_Slot* adjacent_slot = connected_slot();
    if( !adjacent_slot )
        return nullptr;

    return adjacent_slot->node;
}

bool ndbl::nodepropertyview_draw(Node_Property_View* view, View_Detail _detail)
{
    box2d_draw_debug_info(&view->shape);

    if ( !view->state.has_flags(View_Flag_VISIBLE) )
        return false;

    bool            changed            = false;
    Node_Type       node_type          = view->node()->type;

    /*
     * Handle input visibility
     */
    switch (_detail)
    {
        case View_Detail_COMPACT:
        {
            view->show = false;
            view->show |= node_type == Node_Type_VARIABLE;
            view->show |= node_type == Node_Type_VARIABLE_REF;
            break;
        }

        case View_Detail_NORMAL:
        {
            // When untouched, it depends...

            view->show |= node_type == Node_Type_LITERAL;
            view->show |= node_type == Node_Type_VARIABLE;
            view->show |= node_type == Node_Type_VARIABLE_REF;

            // Always show when connected to a variable
            if ( const Node_Slot* slot = view->connected_slot() )
                switch ( slot->node->type )
                {
                    case Node_Type_VARIABLE:
                    case Node_Type_VARIABLE_REF:
                        view->show |= true;
                }

            // Always show properties that have an input slot free
            if (auto* slot = node_find_slot_by_property(view->node(), view->property, Node_Slot::Flag_INPUT))
                view->show |= !slot->is_full();

            view->show |= view->touched;
            break;
        }

        default:
            TOOLS_UNREACHABLE("Unexpected View_Detail_ case (value: %i)\n", _detail);
    }

    // input
    if ( view->show )
    {
        const bool compact_mode = true;
        changed = nodepropertyview_draw_input(view, compact_mode, nullptr);

        if ( ImGui::IsItemFocused() )
        {
            view->show    = false;
            view->touched = false;
        }
    }
    else
    {
        ImGui::Button("", { 8.f, 24.f} );

        if ( ImGui::IsItemClicked(0) )
        {
            get_config()->ui_node_detail = View_Detail_NORMAL;
            view->touched = true;
            view->show    = true;
        }

    }

    if ( ImGuiEx::BeginTooltip() )
    {
        ImGui::Text("%s %s\n", view->property->type->name(), view->property->name.c_str());

        std::string  source_code;
        if( view->property == view->node()->value || node_find_slot_by_property( view->node(), view->property, Node_Slot::Flag_OUTPUT ))
            get_language()->serialize_node(source_code, view->node(), Serialization_Flag_RECURSE);
        else
            get_language()->serialize_property(source_code, view->property);

        ImGui::Text("source: \"%s\"", source_code.c_str());

        ImGuiEx::EndTooltip();
    }

    // Update position and size
    // We want the rectangle to fit the Node_View in height,
    // but we resize it to fit the property input field in width.
    auto* nodeview = componentbag_get<Node_View>(&view->node()->component_bag);

    Rect new_rect  = nodeview->shape.rect(WORLD_SPACE);
    new_rect.min.x = ImGui::GetItemRectMin().x;
    new_rect.max.x = ImGui::GetItemRectMax().x;

    view->shape.set_position(new_rect.top_left(), WORLD_SPACE);
    view->shape.set_size(new_rect.size());

#if DEBUG_DRAW
    ImGuiEx::DebugCircle( spatial_node()->position(), 2.5f, ImColor(0,0,0));
#endif
    return changed;
}

void ndbl::nodepropertyview_reset(Node_Property_View* view)
{
    view->touched = false;
    view->show    = false;
}

float ndbl::nodepropertyview_calc_input_width(const char *buf)
{
    return PROPERTY_INPUT_PADDING + std::max(ImGui::CalcTextSize(buf).x, PROPERTY_INPUT_SIZE_MIN);
}

bool ndbl::nodepropertyview_draw_input(Node_Property_View* view, bool compact_mode, const char* override_label)
{
    Token&              property_token = view->property->token;
    const Node_Slot*    connected_slot = view->connected_slot();
    ImGuiInputTextFlags flags          = ImGuiInputTextFlags_ReadOnly * (connected_slot != nullptr);
    std::string         label;

    // Create a label (everything after ## will not be displayed)
    if ( override_label != nullptr )
        label.append(override_label);
    else
        label.append("##" + view->property->name);

    //
    // Strategy:
    // 1) if property is connected to an identifier, we just display the value in an InputText (as read-only)
    // 2) if property is an identifier, or a literal we allow edition via an InputText, InputDouble/Int or Checkbox

    // 1
    if (view->property->node->type != Node_Type_VARIABLE)
        if ( connected_slot != nullptr )
            switch (connected_slot->node->type)
            {
                case Node_Type_VARIABLE:
                case Node_Type_VARIABLE_REF:
                {
                    char buf[256];
                    const Token &connected_property_token = connected_slot->property->token;
                    snprintf(buf, std::min(connected_property_token.word_len() + 1, sizeof(buf)), "%s",
                             connected_property_token.word());
                    float w = nodepropertyview_calc_input_width(buf);
                    ImGui::PushItemWidth(w);
                    auto* nodeview = componentbag_get<Node_View>(&connected_slot->node->component_bag);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, *nodeview->colors[Color_FILL]);
                    if (ImGui::InputText(label.c_str(), buf, sizeof(buf), flags))
                    {
                        // is ReadOnly
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopItemWidth();
                    return false;
                }
            }

    // 2)

    // Common
    bool changed = false;

    if ( compact_mode )
    {
        std::string token_word = property_token.word_to_string();
        float w = nodepropertyview_calc_input_width(token_word.c_str());
        ImGui::PushItemWidth(w);
    }

    // Per type
    switch ( property_token.m_type )
    {
        case Token_Type::identifier:
        {
            char buf[256];
            snprintf(buf, std::min(property_token.word_len() + 1, sizeof(buf)), "%s", property_token.word());
            flags = 0; // ReadOnly always OFF. ImGuiInputTextFlags_ReadOnly * (connected_slot != nullptr);
            if (ImGui::InputText(label.c_str(), buf, sizeof(buf), flags))
            {
                property_token.word_replace(buf);
                changed = true;
            }
            break;
        }

        case Token_Type::literal_double:
        {

            double value = get_language()->parse_double_or(property_token.word_to_string(), 0);

            if (ImGui::InputDouble(label.c_str(), &value, 0.0, 0.0, "%.6f", flags))
            {
                std::string str;
                get_language()->serialize_double(str, value);
                property_token.word_replace(str.c_str());
                changed = true;
            }
            break;
        }

        case Token_Type::literal_int:
        {
            i32_t value = get_language()->parse_int_or( property_token.word_to_string(), 0);

            if (ImGui::InputInt(label.c_str(), &value, 0, 0, flags))
            {
                std::string str;
                get_language()->serialize_int(str, value);
                property_token.word_replace(str.c_str());
                changed = true;
            }
            break;
        }

        case Token_Type::literal_bool:
        {
            auto str   = property_token.word_to_string();
            bool value = get_language()->parse_bool_or(str, false);

            if (ImGui::Checkbox(label.c_str(), &value))
            {
                str.clear();
                get_language()->serialize_bool(str, value);
                property_token.word_replace(str.c_str());
                changed = true;
            }
            break;
        }

        default:
        {
            char buf[256];
            snprintf(buf, std::min(property_token.word_len() + 1, sizeof(buf)), "%s", property_token.word());

            if (ImGui::InputText(label.c_str(), buf, sizeof(buf), flags))
            {
                property_token.word_replace(buf);
                changed = true;
            }
            break;
        }
    }

    if ( compact_mode )
        ImGui::PopItemWidth();

    return changed;
}

bool ndbl::nodepropertyview_draw_all(const std::vector<Node_Property_View *>& views, View_Detail detail)
{
    bool changed = false;

    if ( !views.empty() )
    {
        for(auto view : views)
        {
            ImGui::SameLine();
            changed |= nodepropertyview_draw(view, detail );
        }
    }

    return changed;
};
