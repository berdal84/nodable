#include <ndbl/core/DirectedEdge.h>
#include <ndbl/core/Node.h>

using namespace ndbl;

DirectedEdge::DirectedEdge(Property * _src, Edge_t _type, Property * _dst)
    : type(_type)
    , prop(_src, _dst)
{
    sanitize(this);
}

DirectedEdge::DirectedEdge(Node* _src, Edge_t _type, Node * _dst)
    : DirectedEdge(_src->get_this_property(), _type, _dst->get_this_property())
{}

DirectedEdge::DirectedEdge(Property * _src, Property * _dst)
    : DirectedEdge(_src, Edge_t::IS_INPUT_OF, _dst)
{}

DirectedEdge::DirectedEdge(Edge_t _type, const Pair<Property *> _pair)
    : DirectedEdge(_pair.src, _type, _pair.dst)
{}

bool DirectedEdge::is_connected_to(Node* _node)const
{
    return prop.src->get_owner() == _node || prop.dst->get_owner() == _node;
}

bool DirectedEdge::operator==(const DirectedEdge& other) const
{
    return this->type == other.type && this->prop == other.prop;
}

void DirectedEdge::sanitize(DirectedEdge* edge)
{
    NDBL_EXPECT(edge->prop.src != nullptr, "edge->prop.src is nullptr");
    NDBL_EXPECT(edge->prop.dst != nullptr, "edge->prop.dst is nullptr");
    NDBL_EXPECT(edge->prop.src != edge->prop.dst, "edge->prop.src and edge->prop.dst are identical");

    if (edge->type == Edge_t::IS_PREDECESSOR_OF ) // we never want this type of edge in our database
    {
        edge->type = Edge_t::IS_SUCCESSOR_OF;
        edge->prop.swap();
    }
}