#include <iostream>
#include "Node.h"

using namespace std;
using namespace Nodable;

int main(int n, const char** args){

	cout << " -- Nodable v0.1 - by Berdal84 - 2017 --" << endl;

	auto a 		= new Node_Integer(10);
	auto b 		= new Node_Integer(9);
	auto c 		= new Node_Integer();

	auto add 	= new Node_Add(a, b, c);

	cout << "Before evaluation : " << endl;
	cout << a->getValue() << " + " << b->getValue() << " = " << c->getValue() << endl;

	add->evaluate();

	cout << "After evaluation : " << endl;
	cout << a->getValue() << " + " << b->getValue() << " = " << c->getValue() << endl;

	return 0;
}