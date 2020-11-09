
#include "Connector.h"
#include "Node.h"
#include "NodeView.h"

ImVec2 Nodable::Connector::position() const {
    auto m = this->member.lock();
    auto node       = m->getOwner()->as<Node>();
    auto view       = node->getComponent<NodeView>();
    auto position   = view->getConnectorPosition(m->getName(), this->way);
    return position;
}

bool Nodable::Connector::equals(const Nodable::Connector *_other) const {
    return this->member.lock() == _other->member.lock() &&
           this->way == _other->way;
}

Nodable::Connector::Connector(std::weak_ptr<Member> _member, Nodable::Way _way):
        member( std::move(_member) ),
        way(_way) {
}
