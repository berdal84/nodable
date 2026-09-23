#include "Node_Property_View.h"

#include "bdc/Allocators.hpp"
#include "bdc/String_Builder.hpp"
#include "bdc/String.hpp"
#include "Asserts.h"
#include "Flags.h"
#include "Parser.h"
#include "Node.h"
#include "Config.h"
#include "geometry/Rect.h"
#include "geometry/Space.h"
#include "geometry/Vec2.h"
#include "ImGuiEx.h"
#include "Node_View.h"
#include "View_Flags.h"

using namespace ndbl;
using namespace ndbl;

constexpr Vec2  PROPERTY_TOGGLE_BTN_SIZE = Vec2(12.0, 22.0f);
constexpr float PROPERTY_INPUT_PADDING   = 5.0f;
constexpr float PROPERTY_INPUT_SIZE_MIN  = 12.0f;

void ndbl::nodepropertyview_init(Node_Property_View* view, Node_Property* property)
{
    ASSERT(property);
    view->shape    = Vec2{10.f, 10.f}; // can't be 0,0
    view->property = property;
}

void ndbl::nodepropertyview_deinit(Node_Property_View* view)
{
    ASSERT(view->property);
    view->property = nullptr;
}


bool ndbl::nodepropertyview_draw(Node_Property_View* view, View_Detail _detail)
{
    box2d_draw_debug_info(&view->shape);

    if ( HAS_FLAGS(view->flags, View_Flag_HIDDEN) )
    {
        return false;
    }

    bool            changed            = false;
    Node*           node               = view->node();
    
    if( !node )
    {
        return false;
    }

    /*
     * Handle input visibility
     */
    switch (_detail)
    {
        case View_Detail_COMPACT:
        {
            view->show = false;
            view->show |= node->type == Node_Type_VARIABLE;
            view->show |= node->type == Node_Type_VARIABLE_REF;
            break;
        }

        case View_Detail_NORMAL:
        {
            // When untouched, it depends...

            view->show |= node->type == Node_Type_LITERAL;
            view->show |= node->type == Node_Type_VARIABLE;
            view->show |= node->type == Node_Type_VARIABLE_REF;

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
            UNREACHABLE("Unexpected View_Detail_ case (value: %i)\n", _detail);
    }

    // input
    if ( view->show )
    {
        const bool compact_mode = true;
        changed = nodepropertyview_draw_input(view, compact_mode);

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
            config()->ui_node_detail = View_Detail_NORMAL;
            view->touched = true;
            view->show    = true;
        }

    }

    if ( ImGuiEx::BeginTooltip() )
    {
        ImGui::Text("%s %s\n", view->property->type->name.c_str(), view->property->name.c_str());

        Serializer_Context serializer;
        serializer_init(serializer);
        if( view->property == view->node()->value || node_find_slot_by_property( view->node(), view->property, Node_Slot::Flag_OUTPUT ))
            serialize_node(serializer, view->node(), Serialization_Flag_RECURSE);
        else
            serialize_property(serializer, view->property);

        ImGui::Text("source: \"%s\"", serializer_build_tstring(serializer).c_str());
        serializer_deinit(serializer);
        ImGuiEx::EndTooltip();
    }

    // Update position and size
    // We want the rectangle to fit the Node_View in height,
    // but we resize it to fit the property input field in width.
    ASSERT(node->view);
    Rect new_rect  = node->view->shape.rect(WORLD_SPACE);
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

float ndbl::nodepropertyview_calc_input_width(const bdc::String& buf)
{
    return PROPERTY_INPUT_PADDING + std::max(ImGui::CalcTextSize(buf.c_str()).x, PROPERTY_INPUT_SIZE_MIN);
}

bool ndbl::nodepropertyview_draw_input(Node_Property_View* view, bool compact_mode, bdc::String override_label)
{
    Token&              property_token = view->property->token;
    const Node_Slot*    connected_slot = view->connected_slot();
    ImGuiInputTextFlags flags          = ImGuiInputTextFlags_ReadOnly * (connected_slot != nullptr);

    bdc::String label;

    // Create a label (everything after ## will not be displayed)
    if ( override_label.size )
        label = override_label;
    else
        label = bdc::string_tprintf("##%s", view->property->name.c_str() );

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
                    const Token &connected_property_token = connected_slot->property->token;
                    bdc::String buf = bdc::string_copy( connected_property_token.word_view());
                    float w = nodepropertyview_calc_input_width(buf);
                    ImGui::PushItemWidth(w);
                    Node_View* nodeview = connected_slot->node->view;
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, *nodeview->colors[Color_FILL]);

                    
                    if (ImGui::InputText(label.c_str(), buf.data, buf.size, flags))
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
        float w = nodepropertyview_calc_input_width( property_token.word_view().c_str() );
        ImGui::PushItemWidth(w);
    }

    Parser_Context parser;
    parser_init(parser);
    
    Serializer_Context serializer;
    serializer_init(serializer);

    // Per type
    switch ( property_token.type )
    {
        case Token_Type_identifier:
        {
            bdc::String buf = bdc::string_tcopy(property_token.word_view().c_str());
            flags = 0; // ReadOnly always OFF. ImGuiInputTextFlags_ReadOnly * (connected_slot != nullptr);
            if (ImGui::InputText(label.c_str(), buf.data, buf.size, flags))
            {
                property_token.replace_word(buf);
                changed = true;
            }
            break;
        }

        case Token_Type_literal_double:
        {

            String word  = property_token.word_view();
            double value = parse_double_or(parser, word, 0);

            if (ImGui::InputDouble(label.c_str(), &value, 0.0, 0.0, "%.6f", flags))
            {
                bdc::String str = serialize_double(serializer, value);
                property_token.replace_word( str.c_str());
                changed = true;
            }
            break;
        }

        case Token_Type_literal_int:
        {
            String word = property_token.word_view();
            i32_t value = parse_int_or( parser, word, 0);

            if (ImGui::InputInt(label.c_str(), &value, 0, 0, flags))
            {
                bdc::String str = serialize_int(serializer, value);
                property_token.replace_word(str.c_str());
                changed = true;
            }
            break;
        }

        case Token_Type_literal_bool:
        {
            bdc::String word = property_token.word_view();
            bool value = parse_bool_or(parser, word, false);

            if (ImGui::Checkbox(label.c_str(), &value))
            {
                bdc::String new_word = serialize_bool(serializer, value);
                property_token.replace_word( new_word );
                changed = true;
            }
            break;
        }

        default:
        {
            bdc::String value_str = string_tcopy( property_token.word_view() );

            if (ImGui::InputText(label.c_str(), value_str.data, value_str.size, flags))
            {
                property_token.replace_word(value_str);
                changed = true;
            }
            break;
        }
    }

    parser_deinit(parser);
    serializer_deinit(serializer);

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