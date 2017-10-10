#include <iostream>
#include "Node.h"

using namespace std;
using namespace Nodable;

int main(int n, const char** args){
	cout << "Hello World" << endl;

	Node* node1 = new Node();
	
	node1->addSlot(2017);
	node1->addSlot("Coucou");
	
	cout << *node1;


	return 0;
}