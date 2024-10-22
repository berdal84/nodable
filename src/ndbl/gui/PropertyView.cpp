#include "PropertyView.h"

#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/Interpreter.h"
#include "NodeView.h"
#include "Config.h"

using namespace ndbl;
using namespace tools;

constexpr Vec2  PROPERTY_TOGGLE_BTN_SIZE = Vec2(12.0, 22.0f);
constexpr float PROPERTY_INPUT_PADDING   = 5.0f;
constexpr float PROPERTY_INPUT_SIZE_MIN  = 12.0f;

PropertyView::PropertyView(Property* _property )
: m_property(_property)
, show(false)
, touched(false)
, m_view_state(10.f, 10.f)
{
}

void PropertyView::reset()
{
    touched    = false;
    show = false;
}

Property* PropertyView::get_property() const
{
    return m_property;
}

Node* PropertyView::get_node() const
{
    return m_property->owner();
}

bool PropertyView::has_input_connected() const
{
    return get_node()->has_input_connected( m_property );
}

Slot* PropertyView::get_connected_slot() const
{
    const Slot* input_slot = get_node()->find_slot_by_property( m_property, SlotFlag_INPUT );
    if( !input_slot )
        return nullptr;

    return input_slot->first_adjacent();
}

VariableNode* PropertyView::get_connected_variable() const
{
    Slot* adjacent_slot = get_connected_slot();
    if( !adjacent_slot )
        return nullptr;

    return cast<VariableNode>(adjacent_slot->node);
}

bool PropertyView::draw(ViewDetail _detail)
{
    m_view_state.box.draw_debug_info();

    if ( !m_view_state.visible )
        return false;

    bool            changed            = false;
    Property*       property           = get_property();
    Node*           node               = get_node();
    NodeType        node_type          = node->type();
    bool            was_visited_by_interpreter = get_interpreter()->was_visited( node );

    /*
     * Handle input visibility
     */
    if ( _detail == ViewDetail::MINIMALIST )
    {
        this->show = false;
        this->show |= node_type == NodeType_VARIABLE;
        this->show |= node_type == NodeType_VARIABLE_REF;
    }
    else
    {
        // When untouched, it depends...

        this->show |= node_type == NodeType_LITERAL;
        this->show |= node_type == NodeType_VARIABLE;
        this->show |= node_type == NodeType_VARIABLE_REF;

        // During debugging we want to see everything if we visited this node
        this->show |= was_visited_by_interpreter;
        // Always show when connected to a variable
        if ( const Slot* connected_slot = get_connected_slot() )
            switch ( connected_slot->node->type() )
            {
                case NodeType_VARIABLE:
                case NodeType_VARIABLE_REF:
                    this->show |= true;
            }

        // Always show properties that have an input slot free
        if (auto* slot = node->find_slot_by_property(property, SlotFlag_INPUT))
            this->show |= !slot->is_full();

        this->show |= this->touched;
    }

    // input
    if ( this->show )
    {
        const bool compact_mode = true;
        changed = PropertyView::draw_input(this, compact_mode, nullptr);

        if ( ImGui::IsItemFocused() )
        {
            this->show    = false;
            this->touched = false;
        }
    }
    else
    {
        ImGui::Button("", { 8.f, 24.f} );

        if ( ImGui::IsItemClicked(0) )
        {
            get_config()->ui_node_detail = ViewDetail::NORMAL;
            this->touched = true;
            this->show    = true;
        }

    }

    if ( ImGuiEx::BeginTooltip() )
    {
        ImGui::Text("%s %s\n", property->get_type()->get_name(), property->name().c_str());

        std::string  source_code;
        if( property == node->value() || node->find_slot_by_property( property, SlotFlag_OUTPUT ))
            get_language()->_serialize_node( source_code, node, SerializeFlag_RECURSE );
        else
            get_language()->serialize_property(source_code, property );

        ImGui::Text("source: \"%s\"", source_code.c_str());

        ImGuiEx::EndTooltip();
    }

    // Memorize new size and position fo this property
    const Vec2 new_size = ImGui::GetItemRectSize();
    const Vec2 new_pos  = ImGui::GetItemRectMin() + ImGui::GetItemRectSize() * 0.5f;
    box()->set_pos( new_pos, WORLD_SPACE ); // GetItemRectMin is in SCREEN_SPACE
    box()->set_size({new_size.x, node->get_component<NodeView>()->box()->size().y}); // We always want the box to fit with the node, it's easier to align things on it

#if DEBUG_DRAW
    ImGuiEx::DebugCircle( rect.center(), 2.5f, ImColor(0,0,0));
#endif
    return changed;
}

float PropertyView::calc_input_width(const char *buf)
{
    return PROPERTY_INPUT_PADDING + std::max(ImGui::CalcTextSize(buf).x, PROPERTY_INPUT_SIZE_MIN);
}

bool PropertyView::draw_input(PropertyView* _view, bool _compact_mode, const char* _override_label)
{
    Property*           property       = _view->get_property();
    Token&              property_token = property->token();
    const Slot*         connected_slot = _view->get_connected_slot();
    ImGuiInputTextFlags flags          = ImGuiInputTextFlags_ReadOnly * (connected_slot != nullptr);
    std::string         label;

    // Create a label (everything after ## will not be displayed)
    if ( _override_label != nullptr )
        label.append(_override_label);
    else
        label.append("##" + property->name());

    //
    // Strategy:
    // 1) if property is connected to an identifier, we just display the value in an InputText (as read-only)
    // 2) if property is an identifier, or a literal we allow edition via an InputText, InputDouble/Int or Checkbox

    // 1
    if (property->owner()->type() != NodeType_VARIABLE)
        if ( connected_slot != nullptr )
            switch (connected_slot->node->type())
            {
                case NodeType_VARIABLE:
                case NodeType_VARIABLE_REF:
                {
                    char buf[256];
                    const Token &connected_property_token = connected_slot->property->token();
                    snprintf(buf, std::min(connected_property_token.word_size() + 1, sizeof(buf)), "%s",
                             connected_property_token.word_ptr());
                    float w = calc_input_width(buf);
                    ImGui::PushItemWidth(w);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                          connected_slot->node->get_component<NodeView>()->get_color(Color_FILL));
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

    if ( _compact_mode )
    {
        std::string token_word = property_token.word_to_string();
        float w = calc_input_width(token_word.c_str());
        ImGui::PushItemWidth(w);
    }

    // Per type
    switch ( property_token.m_type )
    {
        case Token_t::identifier:
        {
            char buf[256];
            snprintf(buf, std::min(property_token.word_size() + 1, sizeof(buf)), "%s", property_token.word_ptr());
            flags = 0; // ReadOnly always OFF. ImGuiInputTextFlags_ReadOnly * (connected_slot != nullptr);
            if (ImGui::InputText(label.c_str(), buf, sizeof(buf), flags))
            {
                property_token.word_replace(buf);
                changed = true;
            }
            break;
        }

        case Token_t::literal_double:
        {
            double value = get_language()->to_double(property_token.word_to_string());

            if (ImGui::InputDouble(label.c_str(), &value, 0.0, 0.0, "%.6f", flags)) {
                std::string str;
                get_language()->serialize_double(str, value);
                property_token.word_replace(str.c_str());
                changed = true;
            }
            break;
        }

        case Token_t::literal_int:
        {
            i32_t value = get_language()->to_int( property_token.word_to_string() );

            if (ImGui::InputInt(label.c_str(), &value, 0, 0, flags))
            {
                std::string str;
                get_language()->serialize_int(str, value);
                property_token.word_replace(str.c_str());
                changed = true;
            }
            break;
        }

        case Token_t::literal_bool:
        {
            auto str = property_token.word_to_string();
            bool b = get_language()->to_bool( str );

            if (ImGui::Checkbox(label.c_str(), &b))
            {
                str.clear();
                get_language()->serialize_bool(str, b);
                property_token.word_replace(str.c_str());
                changed = true;
            }
            break;
        }

        case Token_t::literal_string:
        {
            char buf[256];
            snprintf(buf, std::min(property_token.word_size() + 1, sizeof(buf)), "%s", property_token.word_ptr());

            if (ImGui::InputText(label.c_str(), buf, sizeof(buf), flags))
            {
                property_token.word_replace(buf);
                changed = true;
            }
            break;
        }
        default:
            ASSERT(false); // Not implemented yet!
    }

    if ( _compact_mode )
        ImGui::PopItemWidth();

    return changed;
}

bool PropertyView::draw_all(const std::vector<PropertyView *>& views, ViewDetail _detail)
{
    bool changed = false;

    if ( !views.empty() )
    {
        for(auto view : views)
        {
            ImGui::SameLine();
            changed |= view->draw( _detail );
        }
    }

    return changed;
};
