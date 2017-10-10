#pragma once
#include "vector"
#include "string.h"		// for memcpy
#include "stdlib.h"		// for size_t
#include "iostream"

namespace Nodable{

	enum SlotType_{
		SlotType_Integer,
		SlotType_String,
		SlotType_COUNT
	};

	struct Slot{

		Slot(int _val)
		{
			size 	= 4;
			value 	= new char[size];
			memcpy(&value, (const void*)&_val, size);			
			type 	= SlotType_Integer;
		}

		Slot(const char* _val)
		{
			size 	= strlen(_val);
         	value 	= new char[size];
			memcpy(&value, (const void*)&_val, size);			
			type 	= SlotType_String;
		}

		~Slot(){}
		void* 		value;
		size_t 		size;
		SlotType_ 	type;
	};

	typedef struct Slot Slot;

	class Node{
	public:
		Node();
		~Node();
		void						addSlot(int _val);
		void						addSlot(const char* _val);
		const std::vector<Slot*>& 	getSlots()const;

		friend std::ostream& operator<<(std::ostream& _stream, Node& _node){	
			for(auto slot : _node.getSlots()){
				_stream << slot->value << std::endl;
			}
			//_stream << "coucou";
			return _stream;
		}
	private:
		std::vector<Slot*> slots;
	};
}