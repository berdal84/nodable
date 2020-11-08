
#include "Connector.h"
#include "Node.h"
#include "NodeView.h"

ImVec2 Nodable::Connector::position() const {
    auto node       = this->member->getOwner()->as<Node>();
    auto view       = node->getComponent<NodeView>();
    auto position   = view->getConnectorPosition(this->member->getName(), this->way);
    return position;
}

bool Nodable::Connector::equals(const Nodable::Connector *_other) const {
    return this->member == _other->member &&
           this->way == _other->way;
}

Nodable::Connector::Connector(Nodable::Member *_member, Nodable::Way _way) :
        member(_member),
        way(_way) {
}
