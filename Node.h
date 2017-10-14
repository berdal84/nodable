#pragma once
#include "vector"
#include "string.h"		// for memcpy
#include "stdlib.h"		// for size_t
#include "iostream"

namespace Nodable{
	class Node;
	class Node_Integer;
	class Node_Add;
	class Node_Tag;
	class Node_Context;
	class Node_String;

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

	class Node_String : public Node{
	public:
		Node_String(const char* _value="");
		~Node_String();
		void setValue(const char* /*value*/);
		const char* getValue()const;
	private:
		std::string value;
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

	class Node_Tag : public Node{
	public:
		Node_Tag(Node_Context* _context, const char* _name, Node* _value);
		~Node_Tag();
		Node* 			getValue()const;
		const char* 	getName()const;
	private:
		std::string 	name;
		Node* 			value;
		Node_Context* 	context;
	};

	class Node_Context : public Node {
	public:
		Node_Context(const char* /*name*/);
		~Node_Context();
		void 		add(Node_Tag*);
		Node_Tag* 	find(const char*);
	private:		
		std::vector<Node_Tag*> tags;
		std::string 	name;
	};
}