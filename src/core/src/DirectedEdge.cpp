#include <nodable/core/DirectedEdge.h>
#include <nodable/core/Node.h>

using namespace ndbl;

DirectedEdge::DirectedEdge(Edge_t _type, Node* _src, Node * _dst): type(_type), prop(_src->get_this_property(), _dst->get_this_property()){ sanitize(); }
DirectedEdge::DirectedEdge(Edge_t _type, Property * _src, Property * _dst): type(_type), prop(_src, _dst){ sanitize(); }
DirectedEdge::DirectedEdge(Property * _src, Property * _dst): DirectedEdge(Edge_t::IS_INPUT_OF, _src, _dst) {}
DirectedEdge::DirectedEdge(Property * _src, Edge_t _type, Property * _dst): type(_type), prop(_src, _dst){ sanitize(); }
DirectedEdge::DirectedEdge(Edge_t _type, const Pair<Property *> _pair) : type(_type), prop(_pair){ sanitize(); }
DirectedEdge::DirectedEdge(Node * _src, Edge_t _type, Node * _dst) : DirectedEdge(_type, _src, _dst) {}

bool DirectedEdge::is_about(Node* _node)const
{
    return prop.src->get_owner() == _node || prop.dst->get_owner() == _node;
}

bool DirectedEdge::operator==(const DirectedEdge& other) const
{
    return this->type == other.type && this->prop == other.prop;
}

void DirectedEdge::sanitize()
{
    NDBL_EXPECT(prop.src != nullptr, "prop.src is nullptr");
    NDBL_EXPECT(prop.dst != nullptr, "prop.dst is nullptr");
    NDBL_EXPECT(prop.src != prop.dst, "prop.src and prop.dst are identical");

    if (type == Edge_t::IS_PREDECESSOR_OF ) // we never want this type of relation in our database
    {
        type = Edge_t::IS_SUCCESSOR_OF;
        prop.swap();
    }
}