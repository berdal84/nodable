#include "Node.h"
#include "iostream"

using namespace Nodable;
using namespace std;

Node::Node(){}
Node::~Node(){}

void	Node::addSlot(int _val){
	cout << "Adding an integer slot to the node : " << _val << endl;

	Slot* slot = new Slot(_val);
	this->slots.push_back(slot);
}

void	Node::addSlot(const char* _val){
	cout << "Adding a string slot to the node : " << _val << endl;
	Slot* slot = new Slot(_val);
	this->slots.push_back(slot);
}

const std::vector<Slot*>& Node::getSlots()const{
	return this->slots;
}