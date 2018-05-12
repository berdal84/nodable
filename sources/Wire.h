#include "Nodable.h"
#include "Node.h"

#include <vector>
#include <string>

namespace Nodable
{


	class Wire
	{
	public:
		
		enum State_
		{
			State_Disconnected,
			State_Misconnected,
			State_Connected
		};

		Wire();
		~Wire();

		void        setSource    (Node*, const char*);
		void        setTarget    (Node*, const char*);

		State_      getState     ()const{return state;}
		Node*       getSource    ()const{return source;}
		Node*       getTarget    ()const{return target;}
		std::string getSourceSlotTypeAsString    ()const{return source->getMember(sourceSlot).getTypeAsString();}
		std::string getTargetSlotTypeAsString    ()const{return target->getMember(targetSlot).getTypeAsString();}
		const char* getSourceSlot()const{return sourceSlot.c_str();}
		const char* getTargetSlot()const{return targetSlot.c_str();}
		WireView*   getView      ()const{return view;}

		/* transfert the value from the source to the target */
		void        transmitData();
	private:
		Node*       source       = nullptr;
		std::string sourceSlot   = "";

		Node*       target       = nullptr;
		std::string targetSlot   = "";

		WireView*   view         = nullptr;
		State_      state        = State_Disconnected;

		static std::vector<Wire*> s_wires;
	};
}