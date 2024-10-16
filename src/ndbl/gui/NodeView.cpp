#include "NodeView.h"

#include <algorithm> // for std::max
#include <cmath> // for sinus
#include <vector>

#include "tools/core/math.h"
#include "ndbl/core/GraphUtil.h"
#include "ndbl/core/FunctionNode.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/language/Nodlang.h"

#include "Config.h"
#include "Event.h"
#include "Physics.h"
#include "PropertyView.h"
#include "GraphView.h"
#include "SlotView.h"
#include "tools/gui/Config.h"
#include "ndbl/core/Interpreter.h"

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 0
#endif

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<NodeView>("NodeView")
        .extends<NodeComponent>();
}

constexpr Vec2 DEFAULT_SIZE             = Vec2(10.0f, 35.0f);
constexpr Vec2 DEFAULT_POS              = Vec2(500.0f, -1.0f);
constexpr Vec4 DEFAULT_COLOR            = Vec4(1.f, 0.f, 0.f);
constexpr bool PIXEL_PERFECT            = true; // round positions for drawing only
constexpr float PROPERTY_INPUT_PADDING  = 5.0f;
constexpr float PROPERTY_INPUT_SIZE_MIN = 10.0f;
constexpr Vec2 PROPERTY_TOGGLE_BTN_SIZE = Vec2(10.0, 25.0f);

NodeView::NodeView()
        : NodeComponent()
        , m_base_view()
        , m_colors({&DEFAULT_COLOR})
        , m_opacity(1.0f)
        , m_expanded(true)
        , m_pinned(false)
        , m_property_view_this(nullptr)
        , m_property_views__all()
        , m_hovered_slotview(nullptr)
        , m_last_clicked_slotview(nullptr)
{
    m_base_view.set_pos(DEFAULT_POS, PARENT_SPACE);
    m_base_view.set_size(DEFAULT_SIZE);
}

NodeView::~NodeView()
{
    for(auto& [_, each] : m_property_views__all )
        delete each;

    for(auto* each : m_slot_views )
        delete each;
}

std::string NodeView::get_label()
{
    Config* cfg = get_config();
    if (cfg->ui_node_detail == ViewDetail::MINIMALIST )
    {
        // I always add an ICON_FA at the beginning of any node label string (encoded in 4 bytes)
        return m_short_label;
    }
    return m_label;
}

void NodeView::set_owner(Node* node)
{
    NodeComponent::set_owner(node);

    if( node == nullptr )
    {
        return;
    }

    Config* cfg = get_config();

    // 1. Create Property views
    //-------------------------

    // Reserve
    for(auto& [_, property_view] : m_property_views__all)
        delete property_view;

    m_property_views__all.clear();
    m_property_views__out_strictly.clear();
    m_property_views__inout_strictly.clear();
    m_property_views__in_strictly.clear();
    m_property_views__in.clear();
    m_property_views__out.clear();

    for (Property* property : node->get_props() )
    {
        // Create view
        auto property_view = new PropertyView(property);
        m_base_view.add_child(property_view);
        m_property_views__all.emplace(property, property_view);

        // Indexing
        if ( property->has_flags(PropertyFlag_IS_THIS) )
        {
            m_property_view_this = property_view;
            continue;
        }

        bool has_in  = get_node()->find_slot_by_property(property, SlotFlag_INPUT );
        bool has_out = get_node()->find_slot_by_property(property, SlotFlag_OUTPUT );

        if ( has_in)  m_property_views__in.push_back(property_view);
        if ( has_out) m_property_views__out.push_back(property_view);

        if ( has_in && has_out )
            m_property_views__inout_strictly.push_back(property_view);
        else if ( has_in )
            m_property_views__in_strictly.push_back(property_view);
        else if ( has_out )
            m_property_views__out_strictly.push_back(property_view);
    }

    // 2. Create a SlotView per slot
    //------------------------------

    std::unordered_map<SlotFlags, u8_t> next_index_per_type{
        {SlotFlag_NEXT, 0},
        {SlotFlag_PREV, 0},
        {SlotFlag_INPUT, 0},
        {SlotFlag_OUTPUT, 0},
        {SlotFlag_CHILD, 0},
        {SlotFlag_PARENT, 0},
    };

    for(auto* each : m_slot_views )
        delete each;
    m_slot_views.clear();

    for(Slot* slot : get_node()->slots() )
    {
        Vec2      slot_align;
        ShapeType slot_shape         = ShapeType_CIRCLE;
        SlotFlags slot_type_n_order  = slot->type_and_order();
        u8_t      slot_index         = next_index_per_type[slot_type_n_order]++;

        switch ( slot_type_n_order )
        {
            case SlotFlag_INPUT:
                slot_align = TOP;
                break;

            case SlotFlag_PREV:
                slot_align = TOP_LEFT;
                slot_shape = ShapeType_RECTANGLE;
                break;

            case SlotFlag_OUTPUT:
                if (slot->is_this())
                    slot_align = LEFT;
                else
                    slot_align = BOTTOM;
                break;

            case SlotFlag_NEXT:
                slot_align = BOTTOM_LEFT;
                slot_shape = ShapeType_RECTANGLE;
                break;

            default:
                continue; // skipped
        }

        auto* slotview = new SlotView(slot, slot_align, slot_shape, slot_index);
        m_base_view.add_child(slotview);
        m_slot_views.push_back(slotview);
    }

    // 3. Update label
    //----------------

    update_labels_from_name(node );
    node->on_name_change().connect([=](Node* _node)
    {
        this->update_labels_from_name(_node);
    });

    // 4. Update fill color
    //---------------------

    // note: We pass color by address to be able to change the color dynamically
    set_color( &cfg->ui_node_fill_color[node->type()] );
}

void NodeView::update_labels_from_name(const Node* _node)
{
    // Label
    // For a variable, label must be the type
    if ( _node->type() == NodeType_VARIABLE )
        m_label = reinterpret_cast<const VariableNode *>(_node)->get_type()->get_name();
    else
        m_label = _node->get_name();

    // Short label
    constexpr size_t label_max_length = 10;
    if ( m_label.size() <= label_max_length )
        m_short_label = m_label;
    else
        m_short_label = m_label.substr(0, label_max_length) + "..";
}

void NodeView::translate(const tools::Vec2 &_delta)
{
    m_base_view.translate(_delta);
}

void NodeView::translate_ex(const tools::Vec2 &_delta, NodeViewFlags flags)
{
    ASSERT(flags != NodeViewFlag_NONE ) // You are using translate_ex with no flags?

    m_base_view.translate(_delta);

    for(auto each_input: get_adjacent(SlotFlag_INPUT)  )
    {
        if( !each_input )
            continue;

        if( !each_input->m_base_view.selected  )
            if( flags & NodeViewFlag_EXCLUDE_UNSELECTED )
                continue;

        if( each_input->m_pinned )
            if((flags & NodeViewFlag_WITH_PINNED) == 0)
                continue;

        if( each_input->get_node()->should_be_constrain_to_follow_output( this->get_node() ) )
        {
            each_input->translate_ex(_delta, flags);
        }
    }
}

void NodeView::arrange_recursively(bool _smoothly)
{
    for (auto each_input: get_adjacent(SlotFlag_INPUT) )
    {
        if ( !each_input->m_pinned && each_input->get_node()->should_be_constrain_to_follow_output( this->get_node() ))
        {
            each_input->arrange_recursively();
        }
    }

    for (auto each_child: get_adjacent(SlotFlag_CHILD)  )
    {
        each_child->arrange_recursively();
    }

    // Force an update of input nodes with a delta time extra high
    // to ensure all nodes will be well-placed in a single call (no smooth moves)
    if ( !_smoothly )
    {
        update(float(1000));
    }

    m_pinned = false;
}

bool NodeView::update(float _deltaTime)
{
    if(m_opacity != 1.0f)
    {
        lerp(m_opacity, 1.0f, 10.0f * _deltaTime);
    }

    Config* cfg = get_config();
    const Vec2 nodeview_halfsize = m_base_view.get_size() * 0.5f;

    for(SlotView* slot_view  : m_slot_views )
    {
        slot_view->visible = false;

        const Slot& slot = slot_view->get_slot();

        if (slot.capacity() == 0)
            continue;

        if (slot.type() == SlotFlag_TYPE_CODEFLOW )
            if (!get_node()->is_instruction())
                if (!get_node()->can_be_instruction() )
                    continue;

        slot_view->visible = true;

        switch ( slot_view->get_shape())
        {
            case ShapeType_CIRCLE:
            {
                // Circle are snapped vertically on their property view, except for the "this" property.

                const Vec2 half_size{ cfg->ui_slot_circle_radius() };
                Rect slot_rect{-half_size, half_size};

                if( slot.type() == SlotFlag_TYPE_VALUE && slot.get_property()->has_flags(PropertyFlag_IS_THIS) )
                {
                    slot_rect.translate( m_base_view.get_pos() + m_base_view.get_size() * slot_view->get_align() * Vec2{0.5f} );
                }
                else
                {
                    auto property_view = m_property_views__all.at(slot.get_property() );
                    Rect property_rect = property_view->get_rect();
                    slot_rect.translate( property_rect.center() + property_rect.size() * slot_view->get_align() * Vec2{0.5f} );
                }
                slot_view->set_pos(slot_rect.center());
                slot_view->set_size(slot_rect.size());
                break;
            }

            case ShapeType_RECTANGLE:
            {
                // Rectangles are always on top/bottom
                const Vec2 half_size = cfg->ui_slot_rectangle_size*0.5f;
                Rect slot_rect{-half_size, half_size};
                slot_rect.translate(m_base_view.get_pos());
                slot_rect.translate_x( 2.f * cfg->ui_slot_gap + (cfg->ui_slot_rectangle_size.x + cfg->ui_slot_gap) * float(slot_view->get_index()) );
                slot_rect.translate_y(slot_view->get_align().y * cfg->ui_slot_rectangle_size.y * 0.5f );
                slot_rect.translate( slot_view->get_align() * nodeview_halfsize );
                slot_view->set_pos(slot_rect.center());
                slot_view->set_size(slot_rect.size());
            }
        }

    }
	return true;
}

bool NodeView::draw()
{
    m_base_view.draw();

    Config*     cfg       = get_config();
	bool        changed   = false;
    Node*       node      = get_node();

    auto draw_properties = [&](const std::vector<PropertyView*>& views)
    {
        if ( views.empty() )
            return;

        ImGui::BeginGroup();
        for(auto view : views)
        {
            ImGui::SameLine();
            changed |= _draw_property_view( view, cfg->ui_node_detail );
        }
        ImGui::EndGroup();
    };

    ASSERT(node != nullptr);

    m_hovered_slotview      = nullptr; // reset every frame
    m_last_clicked_slotview = nullptr; // reset every frame

    // Draw background slots (rectangles)
    for( SlotView* slot_view: m_slot_views )
        if ( slot_view->get_shape() == ShapeType_RECTANGLE)
            draw_slot(slot_view);

	// Begin the window
	//-----------------
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_opacity);
    Rect screen_rect = m_base_view.get_rect();
    screen_rect.translate( ImGui::GetWindowPos() );
    if ( PIXEL_PERFECT )
    {
        screen_rect.min = Vec2::round( screen_rect.min );
        screen_rect.max = Vec2::round( screen_rect.max );
    }
    ImGui::SetCursorScreenPos(screen_rect.top_left() ); // start from th top left corner
	ImGui::PushID(this);


	// Draw the background of the Group
    Vec4 border_color = cfg->ui_node_borderColor;
    if ( m_base_view.selected )
    {
        border_color = cfg->ui_node_borderHighlightedColor;
    }
    else if (node->is_instruction())
    {
        border_color = cfg->ui_node_fill_color[NodeType_DEFAULT];
    }

    float border_width = cfg->ui_node_borderWidth;
    if( node->is_instruction() )
    {
        border_width *= cfg->ui_node_instructionBorderRatio;
    }

    DrawNodeRect(
            screen_rect,
            get_color( Color_FILL ),
            cfg->ui_node_borderColor,
            cfg->ui_node_shadowColor,
            border_color,
            m_base_view.selected,
            5.0f,
            border_width );

    // Add an invisible just on top of the background to detect mouse hovering
	ImGui::SetCursorScreenPos(screen_rect.top_left());
    bool is_rect_hovered = !ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringRect(screen_rect.min, screen_rect.max);
    Vec2 new_screen_pos = screen_rect.top_left()
                          + Vec2{ cfg->ui_node_padding.x, cfg->ui_node_padding.y} // left and top padding.
                          + Vec2{cfg->ui_slot_circle_radius(), 0.0f}; // space for "this" left slot
    ImGui::SetCursorScreenPos(new_screen_pos);

	// Draw the window content
	//------------------------

    ImGui::BeginGroup();
    ImGui::Dummy({1.f, 1.f}); // Without this, drawing doesn't work (size issues), TODO: understand why and remove this

    ImGui::SameLine(); draw_properties(m_property_views__out_strictly);

    std::string label = get_label().empty() ? " " : get_label();                        // ensure a 1 char width, to be able to grab it
    if ( !m_expanded )
    {
        // symbolize the fact node view is not expanded
        //abel.insert(0, "<<");
        label.append(" " ICON_FA_OBJECT_GROUP);
    }

    if ( node->is_binary_operator() )
    {
        label = "";
    }
    // Label is displayed first unless node is an operator

    const char* format = node->type() == NodeType_FUNCTION ? "%s(" : "%s";
    ImGui::SameLine(); ImGui::Text( format, label.c_str() );

    if ( node->type() == NodeType_OPERATOR )
    {
        // TODO: handle unary and ternary operators
        ImGui::SameLine();
        ImGui::BeginGroup();
        size_t count = 0;
        for(auto view : m_property_views__in)
        {
            ImGui::SameLine();
            changed |= _draw_property_view( view, cfg->ui_node_detail );

            // Append the operator label between the first and the second operator
            if (count == 0 && node->is_binary_operator() )
            {
                ImGui::SameLine(); ImGui::Text("%s", get_label().c_str() );
            }
            count++;
        }
        ImGui::EndGroup();
    }
    else
    {
        ImGui::SameLine(); draw_properties(m_property_views__in_strictly);
        ImGui::SameLine(); draw_properties(m_property_views__inout_strictly);
    }

    if ( node->type() == NodeType_FUNCTION )
    {
        ImGui::SameLine(); ImGui::Text(")");
    }


    ImGui::EndGroup();

    // Ends the Window
    //----------------

    // Update box's size according to item's rect
    Vec2 new_size = ImGui::GetItemRectMax();
    new_size += Vec2{ cfg->ui_node_padding.z, cfg->ui_node_padding.w}; // right and bottom padding
    new_size -= screen_rect.top_left();
    new_size.x = std::max( 1.0f, new_size.x );
    new_size.y = std::max( 1.0f, new_size.y );

    m_base_view.set_size(Vec2::round(new_size));

    // Draw foreground slots (circles)
    for( SlotView* slot_view: m_slot_views )
        if ( slot_view->get_shape() == ShapeType_CIRCLE)
            draw_slot(slot_view);

	ImGui::PopStyleVar();
	ImGui::PopID();

    if ( changed )
        get_node()->set_flags( NodeFlag_IS_DIRTY );

    m_base_view.hovered = is_rect_hovered || m_hovered_slotview != nullptr;

	return changed;
}

void NodeView::DrawNodeRect(
    Rect rect,
    Vec4 color,
    Vec4 border_highlight_col,
    Vec4 shadow_col,
    Vec4 border_col,
    bool selected,
    float border_radius,
    float border_width
)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw the rectangle under everything
    ImGuiEx::DrawRectShadow(rect.min, rect.max, border_radius, 4, Vec2(1.0f), shadow_col);
    ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
    draw_list->AddRectFilled(rect.min, rect.max, ImColor(color), border_radius, flags);
    draw_list->AddRect(rect.min + Vec2(1.0f), rect.max, ImColor(border_highlight_col), border_radius, flags, border_width);
    draw_list->AddRect(rect.min, rect.max, ImColor(border_col), border_radius, flags, border_width);

    // Draw an additional blinking rectangle when selected
    if (selected)
    {
        auto alpha   = sin(ImGui::GetTime() * 10.0F) * 0.25F + 0.5F;
        float offset = 4.0f;
        draw_list->AddRect(
            rect.min - Vec2(offset),
            rect.max + Vec2(offset),
            ImColor(1.0f, 1.0f, 1.0f, float(alpha) ),
            border_radius + offset,
            ~0,
            offset / 2.0f
        );
    }
}

bool NodeView::_draw_property_view(PropertyView* _view, ViewDetail _detail)
{
    bool            changed            = false;
    Property*       property           = _view->get_property();
    NodeType        node_type          = get_node()->type();
    VariableNode*   connected_variable = _view->get_connected_variable();
    bool            was_visited_by_interpreter = get_interpreter()->was_visited(get_node());

    /*
     * Handle input visibility
     */
    if ( _view->touched )
    {
        _view->show_input &= was_visited_by_interpreter;
    }
    else if ( _detail == ViewDetail::MINIMALIST)
    {
        _view->show_input = false;
    }
    else if (_detail == ViewDetail::EXHAUSTIVE)
    {
        _view->show_input = true;
    }
    else
    {
        // When untouched, it depends...

        // Always show literals (their property don't have input slot)
        _view->show_input |= node_type == NodeType_LITERAL;
        // Always show variable properties
        _view->show_input |= node_type == NodeType_VARIABLE;
        // During debugging we want to see everything if we visited this node
        _view->show_input |= was_visited_by_interpreter;
        // Always show when connected to a variable
        _view->show_input |= connected_variable != nullptr;
        // Always show properties that have an input slot free
        if (auto* slot = get_node()->find_slot_by_property(property, SlotFlag_INPUT))
            _view->show_input |= !slot->is_full();
    }

    // input
    if ( _view->show_input )
    {
        const bool compact_mode = true;
        changed = NodeView::draw_property_view(_view, compact_mode, nullptr);
    }
    else
    {
        ImGui::Button("", PROPERTY_TOGGLE_BTN_SIZE);

        if ( ImGui::IsItemClicked(0) )
        {
            _view->show_input = !_view->show_input;
            _view->touched = true;
        }
    }

    if ( ImGuiEx::BeginTooltip() )
    {
        ImGui::Text("%s %s\n", property->get_type()->get_name(), property->get_name().c_str());

        std::string  source_code;
        if( property->has_flags(PropertyFlag_IS_THIS) || get_node()->find_slot_by_property( property, SlotFlag_OUTPUT ))
            get_language()->serialize_node( source_code, get_node(), SerializeFlag_RECURSE );
        else
            get_language()->serialize_property(source_code, property );

        ImGui::Text("source: \"%s\"", source_code.c_str());

        ImGuiEx::EndTooltip();
    }

    // memorize property view rect (screen space)
    // enlarge rect to fit node_view top/bottom
    Rect rect = {
            Vec2{ImGui::GetItemRectMin().x, m_base_view.get_rect().min.y} ,
            Vec2{ImGui::GetItemRectMax().x, m_base_view.get_rect().max.y}
    };
    _view->set_pos(rect.center());
    _view->set_size(rect.size());

#if DEBUG_DRAW
    ImGuiEx::DebugCircle( rect.center(), 2.5f, ImColor(0,0,0));
#endif
    return changed;
}

bool NodeView::draw_property_view(PropertyView* _view, bool _compact_mode, const char* _override_label)
{
    Property*           property       = _view->get_property();
    Token&              property_token = property->get_token();
    const Slot*         connected_slot = _view->get_connected_slot();
    ImGuiInputTextFlags flags          = ImGuiInputTextFlags_ReadOnly * (connected_slot != nullptr);
    std::string         label;

    // Create a label (everything after ## will not be displayed)
    if ( _override_label != nullptr )
        label.append(_override_label);
    else
        label.append("##" + property->get_name());

    //
    // Strategy:
    // 1) if property is connected to an identifier, we just display the value in an InputText (as read-only)
    // 2) if property is an identifier, or a literal we allow edition via an InputText, InputDouble/Int or Checkbox

    // 1
    if ( property->get_owner()->type() != NodeType_VARIABLE
        && connected_slot
        && connected_slot->get_property()->get_token().m_type == Token_t::identifier)
    {
        if ( property_token.m_type != Token_t::null )
            LOG_WARNING("NodeView", "A connected property should never be from type Token_t::null")

        char buf[256];
        const Token &connected_property_token = connected_slot->get_property()->get_token();
        snprintf(buf, std::min(connected_property_token.word_size() + 1, sizeof(buf)), "%s",
                 connected_property_token.word_ptr());
        float w = calc_input_width(buf);
        ImGui::PushItemWidth(w);
        ImGui::PushStyleColor(ImGuiCol_FrameBg,
                              connected_slot->get_node()->get_component<NodeView>()->get_color(Color_FILL));
        if (ImGui::InputText(label.c_str(), buf, sizeof(buf), flags)) {
            // is ReadOnly
        }
        ImGui::PopStyleColor();
        ImGui::PopItemWidth();
        return false;
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

        case Token_t::null:
        {
            break;
        }
        default:
        {
            ASSERT(false) // Not implemented yet!
        }
    }

    if ( _compact_mode )
        ImGui::PopItemWidth();

    return changed;
}

float NodeView::calc_input_width(const char *buf)
{
    return PROPERTY_INPUT_PADDING + std::max(ImGui::CalcTextSize(buf).x, PROPERTY_INPUT_SIZE_MIN);
}

bool NodeView::is_inside(NodeView* _other, const Rect& _rect, Space _space)
{
	return Rect::contains(_rect, _other->m_base_view.get_rect(_space) );
}

void NodeView::draw_as_properties_panel(NodeView *_view, bool *_show_advanced)
{
    tools::Config* tools_cfg = tools::get_config();
    Node* node = _view->get_node();
    const float labelColumnWidth = ImGui::GetContentRegionAvail().x / 2.0f;

    auto draw_labeled_property_view = [&](PropertyView* _property_view)
    {
        Property*property = _property_view->get_property();
        // label (<name> (<type>): )
        ImGui::SetNextItemWidth(labelColumnWidth);
        ImGui::Text(
                "%s (%s): ",
                property->get_name().c_str(),
                property->get_type()->get_name());

        ImGui::SameLine();
        ImGui::Text("(?)");
        if ( ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("Source token:\n %s\n", property->get_token().json().c_str());
            ImGuiEx::EndTooltip();
        }
        // input
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        bool edited = NodeView::draw_property_view(_property_view, false, nullptr);
        if ( edited )
            node->set_flags( NodeFlag_IS_DIRTY );
    };

    ImGui::Text("Name:       \"%s\"" , node->get_name().c_str());
    ImGui::Text("Class:      %s"     , node->get_class()->get_name());

    // Draw exposed input properties

    auto draw_properties = [&](const char* title, const std::vector<PropertyView*>& views)
    {
        ImGui::Text("%s:", title);
        ImGui::Separator();
        ImGui::Indent();
        if( views.empty() )
        {
            ImGui::Text("None.");
            ImGui::Separator();
        }
        else
        {
            for (auto& property_view : views )
            {
                draw_labeled_property_view( property_view );
                ImGui::Separator();
            }
        }
        ImGui::Unindent();
    };

    ImGui::Separator();
    draw_properties("Inputs(s)", _view->m_property_views__in_strictly);
    draw_properties("In/Out(s)", _view->m_property_views__inout_strictly);
    ImGui::Separator();
    draw_properties("Output(s)", _view->m_property_views__out_strictly);
    ImGui::Separator();

    if ( tools_cfg->runtime_debug )
    {
        ImGui::Text("DEBUG INFO:" );
        ImGui::Text("Suffix token:\n       %s\n" , node->get_suffix().json().c_str());
        ImGui::Text("can_be_instruction(): %i", node->can_be_instruction() );
        ImGui::Text("is_instruction():     %i", node->is_instruction() );
        // Draw exposed output properties
        if( ImGui::TreeNode("Other Properties") )
        {
            draw_labeled_property_view( _view->m_property_view_this );
            ImGui::TreePop();
        }

        // Components
        if( ImGui::TreeNode("Components") )
        {
            for (const NodeComponent* component : node->get_components() )
            {
                ImGui::BulletText("%s", component->get_class()->get_name());
            }
            ImGui::TreePop();
        }

        if( ImGui::TreeNode("Slots") )
        {
            auto draw_node_list = [](const char *label, const std::vector<Node*> _nodes )
            {
                if( !ImGui::TreeNode(label) )
                {
                    return;
                }

                if ( _nodes.empty() )
                {
                    ImGui::BulletText( "None" );
                }

                for (const Node* each_node : _nodes )
                {
                    ImGui::BulletText("- %s", each_node->get_name().c_str());
                }

                ImGui::TreePop();
            };

            draw_node_list("Inputs:"      , node->inputs() );
            draw_node_list("Outputs:"     , node->outputs() );
            draw_node_list("Predecessors:", node->predecessors() );
            draw_node_list("Successors:"  , node->successors() );
            draw_node_list("Children:"    , node->children() );

            ImGui::TreePop();
        }

        // Physics Component
        if( ImGui::TreeNode("Physics") )
        {
            auto* physics_component = node->get_component<Physics>();
            ImGui::Checkbox("On/Off", &physics_component->is_active);
            for(Physics::Constraint& constraint : physics_component->get_constraints())
            {
                if (ImGui::TreeNode(constraint.name))
                {
                    ImGui::Checkbox("enabled", &constraint.enabled);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        // Scope specific:
        if (Scope* scope = node->get_component<Scope>())
        {
            if( ImGui::TreeNode("Variables") )
            {
                auto vars = scope->variables();
                for (auto var : vars)
                {
                    std::string value = var->property()->get_token().word_to_string();
                    ImGui::BulletText("%s: %s", var->get_name().c_str(), value.c_str() );
                }
                ImGui::TreePop();
            }
        }

        if( ImGui::TreeNode("Misc:") )
        {
            // dirty state
            ImGui::Separator();
            bool b = node->has_flags(NodeFlag_IS_DIRTY);
            ImGui::Checkbox("Is dirty ?", &b);

            // Parent graph
            {
                std::string parentName = "NULL";

                if (Graph* parent_graph = node->get_parent_graph())
                {
                    parentName = "Graph";
                    parentName.append( parent_graph->is_dirty() ? " (dirty)" : "");
                }
                ImGui::Text("Parent graph is \"%s\"", parentName.c_str());
            }

            // Parent
            ImGui::Separator();
            {
                std::string parentName = "NULL";

                if (Node* parent = node->find_parent() )
                {
                    parentName = parent->get_name() + (parent->has_flags(NodeFlag_IS_DIRTY) ? " (dirty)" : "");
                }
                ImGui::Text("Parent node is \"%s\"", parentName.c_str());
            }
            ImGui::TreePop();
        }
    }
    ImGui::Separator();
}

void NodeView::constraint_to_rect(NodeView* _view, const Rect& _rect)
{
	
	if ( !NodeView::is_inside(_view, _rect ))
    {
        Rect shrinked_rect = _rect;
        shrinked_rect.expand( Vec2( -2, -2 ) ); // shrink

		auto view_rect = _view->m_base_view.get_rect();

		auto left  = _rect.min.x - view_rect.min.x;
		auto right = _rect.max.x - view_rect.max.x;
		auto up    = _rect.min.y - view_rect.min.y;
		auto down  = _rect.max.y - view_rect.max.y;

		     if ( left > 0 )  view_rect.translate_x(left );
		else if ( right < 0 ) view_rect.translate_x(right );
			 
			 if ( up > 0 )  view_rect.translate_y(up );
		else if ( down < 0 )view_rect.translate_y(down );

        _view->m_base_view.set_pos(view_rect.center(), PARENT_SPACE);
	}

}

Rect NodeView::get_rect(Space space) const
{
    return m_base_view.get_rect(space);
}

Rect NodeView::get_rect_ex(tools::Space space, NodeViewFlags flags) const
{
    Rect this_rect = m_base_view.get_rect(space);
    if( (flags & NodeViewFlag_WITH_RECURSION) == 0 )
    {
        return this_rect;
    }

    std::vector<Rect> rects;

    if ( m_base_view.visible )
        rects.push_back( this_rect );

    auto visit = [&](NodeView* view)
    {
        if( !view )
            return;
        if( !view->m_base_view.visible )
            return;
        if(view->m_base_view.selected && (flags & NodeViewFlag_EXCLUDE_UNSELECTED) )
            return;
        if( view->m_pinned && (flags & NodeViewFlag_WITH_PINNED ) == 0 )
            return;
        if( view->get_node()->should_be_constrain_to_follow_output( this->get_node() ) )
        {
            Rect rect = view->get_rect_ex(space, flags);
            rects.push_back( rect );
        }
    };

    auto children = get_adjacent(SlotFlag_CHILD);
    std::for_each(children.begin(), children.end(), visit );

    auto inputs   = get_adjacent(SlotFlag_INPUT);
    std::for_each(inputs.begin()  , inputs.end()  , visit );

    Rect result = Rect::bbox(&rects);

#if DEBUG_DRAW
    Rect screen_rect = result;
    screen_rect.translate(get_pos(space) - get_pos(PARENT_SPACE) );
    ImGuiEx::DebugRect(screen_rect.min, screen_rect.max, IM_COL32( 0, 255, 0, 60 ), 2 );
#endif

    return result;
}

Rect NodeView::get_rect(
    const std::vector<NodeView *> &_views,
    Space space,
    NodeViewFlags flags
)
{
    Rect result;
    for (size_t i = 0; i < _views.size(); ++i)
    {
        Rect rect = _views[i]->get_rect_ex(space, flags);
        if ( i == 0 )
            result = rect;
        else
            result = Rect::merge(result, rect);
    }
    return result;
}

std::vector<Rect> NodeView::get_rects(const std::vector<NodeView*>& _in_views, Space space, NodeViewFlags flags)
{
    std::vector<Rect> rects;
    rects.reserve(_in_views.size());
    size_t i = 0;
    while( i < _in_views.size() )
    {
        rects[i] = _in_views[i]->get_rect_ex(space, flags);
        ++i;
    }
    return std::move( rects );
}

void NodeView::set_expanded_rec(bool _expanded)
{
    set_expanded(_expanded);
    for(NodeView* each_child_view : get_adjacent(SlotFlag_CHILD) )
    {
        each_child_view->set_expanded_rec(_expanded);
    }
}

void NodeView::set_expanded(bool _expanded)
{
    m_expanded = _expanded;
    set_inputs_visible(_expanded, true);
    set_children_visible(_expanded, true);
}

void NodeView::set_inputs_visible(bool _visible, bool _recursive)
{
    set_adjacent_visible( SlotFlag_INPUT, _visible, _recursive );
}

void NodeView::set_children_visible(bool _visible, bool _recursive)
{
    set_adjacent_visible( SlotFlag_CHILD, _visible, _recursive );
}

void NodeView::set_adjacent_visible(SlotFlags flags, bool _visible, bool _recursive)
{
    bool has_not_output = get_adjacent(SlotFlag_OUTPUT).empty();
    for( auto each_child_view : get_adjacent(flags) )
    {
        if( _visible || has_not_output || each_child_view->get_node()->should_be_constrain_to_follow_output( get_node() ) )
        {
            if ( _recursive && each_child_view->m_expanded ) // propagate only if expanded
            {
                each_child_view->set_children_visible(_visible, true);
                each_child_view->set_inputs_visible(_visible, true);
            }
            each_child_view->m_base_view.visible = _visible;
        }
    }
}

void NodeView::expand_toggle()
{
    set_expanded(!m_expanded);
}

NodeView* NodeView::substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive)
{
    if( _view == nullptr )
    {
        return _view;
    }

    if( _view->m_base_view.visible )
    {
        return _view;
    }

    Node* parent = _view->get_node()->find_parent();
    if ( !parent )
    {
        return _view;
    }

    NodeView* parent_view = parent->get_component<NodeView>();
    if ( !parent_view )
    {
        return _view;
    }

    if (  _recursive )
    {
        return substitute_with_parent_if_not_visible(parent_view, _recursive);
    }

    return parent_view;
}

std::vector<NodeView*> NodeView::substitute_with_parent_if_not_visible(const std::vector<NodeView*>& _in, bool _recursive)
{
    std::vector<NodeView*> out;
    out.reserve(_in.size()); // Wort but more probable case
    for(auto each : _in)
    {
        auto each_or_substitute = NodeView::substitute_with_parent_if_not_visible(each, _recursive);
        if (each_or_substitute)
        {
            out.push_back(each_or_substitute);
        }
    }
    return std::move(out);
};

void NodeView::expand_toggle_rec()
{
    return set_expanded_rec(!m_expanded);
}

std::vector<NodeView*> NodeView::get_adjacent(SlotFlags flags) const
{
    return GraphUtil::adjacent_components<NodeView>(get_node(), flags);
}

void NodeView::set_color( const Vec4* _color, ColorType _type )
{
    ASSERT(_color != nullptr)
    m_colors[_type] = _color;
}

Vec4 NodeView::get_color( ColorType _type ) const
{
     auto* color = m_colors[_type];
     VERIFY(color != nullptr, "Did you called set_color(...) ?");
     return *color;
}

GraphView *NodeView::get_graph() const
{
    ASSERT(get_node()->get_parent_graph() != nullptr)
    return get_node()->get_parent_graph()->get_view();
}

void NodeView::draw_slot(SlotView* slot_view)
{
    if( slot_view->draw() )
        m_last_clicked_slotview = slot_view;

    if( slot_view->hovered )
    {
        m_hovered_slotview = slot_view; // last wins
    }
}


void NodeView::translate(const std::vector<NodeView*>& _views, const Vec2& delta)
{
    for (auto node_view : _views )
    {
        node_view->translate(delta);
    }
}
