#include "NodeViewConstraint.h"
#include <numeric>
#include "core/ForLoopNode.h"
#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"

using namespace ndbl;
using namespace fw;

NodeViewConstraint::NodeViewConstraint(const char* _name, ConstrainFlags _flags)
: m_flags(_flags)
, m_should_apply(always)
, m_is_active(true)
, m_name(_name)
{
}

/** TODO: move this in a class (not NodeViewConstrain) */
std::vector<Rect> get_rect(const std::vector<NodeView*>& _in_views)
{
    std::vector<Rect> _out;
    for (auto each_target : _in_views )
    {
        Rect rect;
        if( !(each_target->pinned() || !each_target->is_visible) )
        {
            rect = each_target->get_rect( true );
        }
        _out.push_back(rect);
    }
    return std::move(_out);
}

void NodeViewConstraint::apply(float _dt)
{
    // Check if this constrain should apply
    if(!m_is_active && m_should_apply(this)) return;

    // Gather only visible views or their parent (recursively)
    auto pool = Pool::get_pool();
    std::vector<NodeView*> clean_drivers = NodeView::substitute_with_parent_if_not_visible( pool->get( m_drivers ), true );
    std::vector<NodeView*> clean_targets = NodeView::substitute_with_parent_if_not_visible( pool->get( m_targets ), true );

    // If we still have no targets or drivers visible, it's not necessary to go further
    if ( NodeView::none_is_visible(clean_targets)) return;
    if ( NodeView::none_is_visible(clean_drivers)) return;

    // To control visually
    draw_debug_lines( clean_drivers, clean_targets );

    const Config& config = Nodable::get_instance().config;

    switch ( m_flags & ConstrainFlag_LAYOUT_MASK )
    {
        case ConstrainFlag_LAYOUT_MAKE_ROW:
        {
            /*
             * Make a row with targets, and constrain it to be above at the top (or bottom) the driver's bbox
             */
            NodeView*   driver            = clean_drivers[0];
            const bool  align_bbox_bottom = m_flags & ConstrainFlag_ALIGN_BBOX_BOTTOM;
            const float y_direction       = align_bbox_bottom ? 1.0f : -1.0f;
            Vec2 virtual_cursor    = driver->get_position(Space_Local);
            const Node& driver_owner      = *driver->get_owner();
            auto        target_rects = get_rect( clean_targets );

            // Determine horizontal alignment
            //-------------------------------

            Align halign = Align_CENTER;

            // Align right when driver is an instruction without being connected to a predecessor
            if ( driver_owner.is_instruction() && !driver_owner.predecessors().empty() && not align_bbox_bottom )
            {
                halign = Align_END;
            }

            // Determine virtual_cursor.x from alignment
            //----------------------------------

            switch( halign )
            {
                case Align_START:
                {
                    FW_EXPECT(false, "not implemented")
                }

                case Align_END:
                {
                    virtual_cursor.x += driver->get_size().x / 4.0f + config.ui_node_spacing;
                    break;
                }

                case Align_CENTER:
                {
                    float size_x_total = 0.0f;
                    std::for_each( target_rects.begin(), target_rects.end(),[&](auto each ) { size_x_total += each.size().x; });
                    virtual_cursor.x -= size_x_total / 2.0f;
                }
            }

            // Constraint in row:
            //-------------------
            virtual_cursor.y   += y_direction * driver->get_size().y / 2.0f;
            for (int target_index = 0; target_index < clean_targets.size(); target_index++)
            {
                NodeView* each_target = clean_targets[target_index];
                const Node& target_owner = *each_target->get_owner();

                // Guards
                if ( !each_target->is_visible ) continue;
                if ( each_target->pinned() ) continue;
                if ( !target_owner.should_be_constrain_to_follow_output( driver_owner.poolid() ) && !align_bbox_bottom ) continue;

                // Compute new position for this input view
                Rect& target_rect = target_rects[target_index];

                Vec2 relative_pos(
                        target_rect.width() / 2.0f,
                        y_direction * ( target_rect.height() / 2.0f + config.ui_node_spacing)
                );

                if ( align_bbox_bottom ) relative_pos.y += y_direction * config.ui_node_spacing;

                // Add a vertical space to avoid having too much wires aligned on x-axis
                // useful for "for" nodes.
                if( halign == Align_END && clean_targets.size() > 1 )
                {
                    float reverse_y_spacing = float(clean_targets.size() - 1 - target_index) * config.ui_node_spacing * 1.5f;
                    relative_pos.y += y_direction * reverse_y_spacing;
                }

                auto target_physics = target_owner.get_component<Physics>();
                target_physics->add_force_to_translate_to( virtual_cursor + relative_pos + m_offset, config.ui_node_speed, true);
                virtual_cursor.x += target_rect.width() + config.ui_node_spacing;

            }
            break;
        }

        case ConstrainFlag_LAYOUT_DEFAULT:
        {
            NodeView* target = clean_targets[0];
            if (!target->pinned() && target->is_visible )
            {
                Physics& target_physics = *target->get_owner()->get_component<Physics>();

                // TODO: this if/else should be merged (add new flags for ConstrainFlag_ to distinguish)

                if( m_flags & ConstrainFlag_LAYOUT_FOLLOW_WITH_CHILDREN )
                {
                    /*
                    * Constrain the target view (and its children) to follow the drivers' bbox
                    */

                    // compute
                    auto drivers_rect = NodeView::get_rect(clean_drivers, false);

                    auto target_rect  = target->get_rect(true, true);
                    float target_driver_offset = drivers_rect.max.y - target_rect.min.y;
                    Vec2 new_pos;
                    Vec2 target_position = target->get_position(Space_Local);
                    new_pos.x = drivers_rect.tl().x + target->get_size().x * 0.5f ;
                    new_pos.y = target_position.y + target_driver_offset + config.ui_node_spacing;

                    // apply
                    target_physics.add_force_to_translate_to(new_pos + m_offset, config.ui_node_speed, true);
                }
                else
                {
                    /*
                     * Align first target's bbox border left with all driver's bbox border right
                     */
                    Rect drivers_bbox = NodeView::get_rect(clean_drivers, true);
                    Vec2 new_position( drivers_bbox.center()
                                         - Vec2( drivers_bbox.size().x * 0.5f
                                         + config.ui_node_spacing
                                         + target->get_rect().size().x * 0.5f, 0 ));
                    target_physics.add_force_to_translate_to(new_position + m_offset, config.ui_node_speed);
                }
            }
        }
    }
}

void NodeViewConstraint::draw_debug_lines(const std::vector<NodeView*>& _drivers,const std::vector<NodeView*>& _targets )
{
    if( ImGuiEx::debug )
    {
        for (auto each_target: _targets )
        {
            for (auto each_driver: _drivers )
            {
                ImGuiEx::DebugLine(
                        each_driver->get_position( Space_Screen),
                        each_target->get_position( Space_Screen),
                        IM_COL32(0, 0, 255, 30), 1.0f);
            }
        }
    }
}

void NodeViewConstraint::add_target(PoolID<NodeView> _target)
{
    FW_ASSERT( _target );
    m_targets.push_back(_target);
}

void NodeViewConstraint::add_driver(PoolID<NodeView> _driver)
{
    FW_ASSERT( _driver );
    m_drivers.push_back(_driver);
}

void NodeViewConstraint::add_targets(const std::vector<PoolID<NodeView>> &_new_targets)
{
    m_targets.insert(m_targets.end(), _new_targets.begin(), _new_targets.end());
}

void NodeViewConstraint::add_drivers(const std::vector<PoolID<NodeView>> &_new_drivers)
{
    m_drivers.insert(m_drivers.end(), _new_drivers.begin(), _new_drivers.end());
}


auto not_expanded  = [](PoolID<const NodeView> _view ) { return !_view->is_expanded(); };

const NodeViewConstraint::Filter
        NodeViewConstraint::always = [](NodeViewConstraint* _constraint){ return true; };

const NodeViewConstraint::Filter
        NodeViewConstraint::no_target_expanded = [](const NodeViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_targets.cbegin(), _constraint->m_targets.cend(), not_expanded)
           == _constraint->m_targets.cend();
};

const NodeViewConstraint::Filter
        NodeViewConstraint::drivers_are_expanded = [](const NodeViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_drivers.cbegin(), _constraint->m_drivers.cend(), not_expanded)
           == _constraint->m_drivers.cend();
};

void NodeViewConstraint::draw_view()
{
    if( ImGui::TreeNode(m_name) )
    {
        ImGui::Text("Type:     %s", to_string( (ConstrainFlag_)m_flags ));
        ImGui::Text("Drivers:  %zu", m_drivers.size());
        ImGui::Text("Targets:  %zu", m_targets.size());
        ImGui::Checkbox("On/Off", &m_is_active);
        ImGui::TreePop();
    }
}
