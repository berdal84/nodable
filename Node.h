#pragma once
#include "vector"
#include "string.h"		// for memcpy
#include "stdlib.h"		// for size_t
#include "iostream"

namespace Nodable{
	class Node{
	public:
		Node();
		~Node();
	};

	class Node_Integer : public Node{
	public:
		Node_Integer(int _n=0);
		~Node_Integer();
		void setValue(int _n);
		int getValue()const;
	private:
		int value;
	};

	class Node_Add : public Node{
	public:
		Node_Add(Node_Integer* _inputA, Node_Integer* _inputB, Node_Integer* _output);
		~Node_Add();
		void evaluate();
	private:
		Node_Integer* inputA;
		Node_Integer* inputB;
		Node_Integer* output;
	};
}