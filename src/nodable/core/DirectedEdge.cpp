#include "DirectedEdge.h"
#include "Node.h"

using namespace ndbl;

DirectedEdge::DirectedEdge(Property* _src, Edge_t _type, Property* _dst)
    : m_type(_type)
    , m_props(_src, _dst)
{
    sanitize(this);
}

DirectedEdge::DirectedEdge(Property* _src, Property* _dst)
    : DirectedEdge(_src, Edge_t::IS_INPUT_OF, _dst)
{}

DirectedEdge::DirectedEdge(Edge_t _type, const std::pair<Property*, Property*> _pair)
    : DirectedEdge(_pair.first, _type, _pair.second)
{}

DirectedEdge::DirectedEdge(Node* _src, Edge_t _type, Node* _dst)
    : DirectedEdge(_src->as_prop(), _type, _dst->as_prop() )
{}

DirectedEdge::DirectedEdge(ID<Node> _src, Edge_t _type, ID<Node> _dst)
    : DirectedEdge(_src->as_prop(), _type, _dst->as_prop() )
{}

bool DirectedEdge::is_connected_to(const ID<Node> _node )const
{
    return src()->owner() == _node || dst()->owner() == _node;
}

bool DirectedEdge::operator==(const DirectedEdge& other) const
{
    return this->m_type  == other.m_type &&
           this->m_props == other.m_props;
}

void DirectedEdge::sanitize(DirectedEdge* edge)
{
    FW_EXPECT(edge->m_props.first != nullptr, "edge->prop.src is nullptr");
    FW_EXPECT(edge->m_props.second != nullptr, "edge->prop.dst is nullptr");
    FW_EXPECT(edge->m_props.first != edge->m_props.second, "edge->prop.src and edge->prop.dst are identical");

    if (edge->m_type == Edge_t::IS_PREDECESSOR_OF ) // we never want this type of edge in our database
    {
        edge->m_type = Edge_t::IS_SUCCESSOR_OF;
        edge->m_props.swap( edge->m_props );
    }
}

std::pair<Node*, Node*> DirectedEdge::nodes() const
{
    return std::pair<Node*, Node*>(m_props.first->owner().get(), m_props.second->owner().get() );
}