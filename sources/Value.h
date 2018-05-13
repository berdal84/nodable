#pragma once

#include "Nodable.h"    // for constants and forward declarations
#include <string>

namespace Nodable{

	/* 
		The 	// 1 - Draw nodes
	for(auto each : this->nodes)
	{
		if ( each != nullptr)
		{
			auto view = each->getView();

			if (view != nullptr)
			{
				view->draw();
				isAnyItemDragged |= NodeView::GetDragged() == view;
				isAnyItemHovered |= view->isHovered();
			}
		}
	} class is the base class for basic types such as Numbers, Strings or Booleans
	*/
	class Value{
	public:
		Value(Type_ _type = Type_Unknown);
		~Value();

		void        setValue         (const Value&);
		void        setValue         (std::string);
		void        setValue         (const char*);
		void        setValue         (double);
		double      getValueAsNumber ()const;
		std::string getValueAsString ()const;		
		std::string getTypeAsString  ()const;

		Type_       getType          ()const;
		bool        isType           (Type_ _type)const;
		void        setType          (Type_ _type){type = _type;}
		bool        isSet            ()const;	

	private:
		std::string s = "";
		double      d = 0.0F;
		Type_   type = Type_Unknown;
	};
}